#pragma once
#include "stdafx.h"
#include "util.h"
#include "serial.h"


std::vector<std::string> tokenize(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos) pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty())
		{
			tokens.push_back(token);
		}
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}


const std::string id_prefix = "####RealSimGear#";
const std::chrono::seconds max_id_interval(15);
const int baud = 115200;

#if defined(__APPLE__)
// on MacOS we cannot cancel I/O, thus we lower the timeout
const int byte_timeout = 100;
#else
//const int byte_timeout = 500000;
const int byte_timeout = 5000;
#endif

const int max_line_length = 1024;


enum class serial_msg_type { connected, disconnected, command };

struct serial_msg {	
	serial_msg_type type;
	std::string port;
	std::string id;
	std::string model;
	std::string line;
	std::string version;
	std::string hex_id;//20191006 for nex fmt ####RealSimGear#MODEL#ID#Version#HexID

	std::string str() const {
		std::stringstream ss;
		std::string typ;
		switch (type) {
		case serial_msg_type::connected:
			typ = "connected";
			break;
		case serial_msg_type::disconnected:
			typ = "disconnected";
			break;
		case serial_msg_type::command:
			typ = "command";
			break;
		}
		ss << "port=" << port << " id=" << id << " model=" << model 
			<< " version=" << version << " type=" << typ << " line=" << line;
		return ss.str();
	}
};



inline bool starts_with(const std::string &str, const std::string &prefix) {
	return str.substr(0, prefix.length()) == prefix;
}

class serial_watch {
	typedef std::function<void(const serial_msg &)> notify_t;

private:
	notify_t notify;
	std::thread watch_thread;
	std::thread write_thread;
	std::unique_ptr<serial> serial_port;
	std::mutex serial_port_mutex;
	std::atomic<bool> _okay = true;
	std::atomic<bool> _stop = false;
	std::atomic<bool> _err = false;
	std::atomic<bool> initialized = false;
	std::string _id;
	std::string _model;
	std::string _version;
	std::string _hexId;
	mutable std::mutex _model_lock;
	std::deque<std::string> write_queue;
	std::mutex write_queue_mutex;
	std::condition_variable write_queue_cv;
	size_t contain;

	bool handle_id_line(const std::string &line) {
		//logMessage("handle_id_line:%s",line.c_str());
		// id line has the form: ####RealSimGear#MODEL#ID or ####RealSimGear#MODEL#ID#Version
		// First check to see if this is an id line

		/*20191006:New fmt: ####RealSimGear#RealSimGear-GNS530#1#2.5.2#3AABBCCDD11223344*/
		//####RealSimGear#MODEL#ID#Version
		if (!starts_with(line, id_prefix))
			return false;

		std::vector<std::string> tokens = tokenize(line, "#");

		auto model_id_str = line.substr(id_prefix.length());

		if (tokens.size() < 2)
			return false;

		auto model = tokens[1];
		auto id_str = tokens[2];

		// old or new format (with or without version)
		auto version = tokens.size() > 3 ? tokens[3] : "0.0.0";
		auto hexId = tokens.size() > 4 ? tokens[4] : "";
		if (!initialized) {
			{
				initialized = true;
				std::lock_guard lck(_model_lock);
				_model = model;
				_version = version;
				_id = id_str;
				_hexId = hexId;
			}

			serial_msg msg;
			msg.type = serial_msg_type::connected;
			msg.port = port;
			msg.id = this->id();
			msg.model = this->model();
			msg.version = this->version();
			msg.hex_id = this->hexId();
			notify(msg);
		}

		return true;
	}

	void watch(serial &sp) {
		auto last_id_time = std::chrono::steady_clock::now();

		while (!_stop && last_id_time + max_id_interval > std::chrono::steady_clock::now()) {
			try {
				//logMessage("read line from %s",sp.port.c_str());
				auto line = sp.read_line();
				logMessage("%s: received: %s", sp.port.c_str(), line.c_str());

				if (handle_id_line(line)) {
					// id line detected
					last_id_time = std::chrono::steady_clock::now();
					//logMessage("%s: identified as %s with id %s", sp.port.c_str(), _model.c_str(), _id);
				} else if (line.size() > 0) {
					// command detected
					if (!initialized) {
						logMessage("%s: ignoring command before identification", sp.port.c_str());
					} else {
						serial_msg msg;
						msg.type = serial_msg_type::command;
						msg.port = port;
						msg.id = id();
						msg.model = model();
						msg.version = version();
						msg.line = line;
						notify(msg);
					}
				}
			} catch (serial_timeout) {}
		}
	}

	void watch_thread_fn() {
		logMessage("%s: watch thread started", port.c_str());

		try {
			watch(*serial_port);
		} catch (serial_error se) {
			logMessage("%s: read error: %s", port.c_str(), se.msg.c_str());
			_err = true;
		} 
		
		if (initialized) {
			serial_msg msg;
			msg.type = serial_msg_type::disconnected;
			msg.port = port;
			msg.id = id();
			msg.model = model();
			msg.version = version();
			notify(msg);
		}

		{
			std::unique_lock lck2(serial_port_mutex);
			serial_port.reset();
		}

		_okay = false;
		logMessage("%s: watch thread terminated", port.c_str());
	}

	bool get_write_msg(std::string &msg) {
		std::unique_lock lck(write_queue_mutex);
		while (!_stop) {
			if (!write_queue.empty()) {
				msg = write_queue.front();
				return true;
			} 
			write_queue_cv.wait(lck);
		}
		return false;
	}

	void remove_write_msg() {
		std::unique_lock lck(write_queue_mutex);
		write_queue.pop_front();
	}

	void write_thread_fn() {
		try {
			while (!_stop) {
				std::string msg;
				if (get_write_msg(msg)) {
					try {
						std::unique_lock lck(serial_port_mutex);
						if (serial_port)
							serial_port->write(msg);
						remove_write_msg();
					} catch (serial_timeout) {
						log("%s: write time out: %s", port.c_str(), msg.c_str());
					}
				}
			}
		} catch (serial_error se) {
			logMessage("%s: write error: %s", port.c_str(), se.msg.c_str());
			_err = true;
		}

		_okay = false;
		logMessage("%s: write thread terminated", port.c_str());
	}

public:
	const std::string port;

	serial_watch(std::string port, notify_t notify_fn) : port(port), notify(notify_fn) {
		try {
			serial_port = std::make_unique<serial>(port, baud, byte_timeout, max_line_length);

			logMessage("%s: starting watch thread", port.c_str());
			watch_thread = std::thread([this] {watch_thread_fn(); });

			logMessage("%s: starting write thread", port.c_str());
			write_thread = std::thread([this] {write_thread_fn(); });
		} catch (serial_error se) {
			logMessage("%s: error: %s", port.c_str(), se.msg.c_str());
			_err = true;
		}
	}

	~serial_watch() {
		logMessage("%s: requesting watch and write thread stop", port.c_str());
		_stop = true;
		write_queue_cv.notify_all();
		{
			std::unique_lock lck2(serial_port_mutex);
			if (serial_port)
				serial_port->cancel_io();
		}
		if (watch_thread.joinable()) 		
			watch_thread.join();
		if (write_thread.joinable())
			write_thread.join();
		logMessage("%s: watch and write threads stopped", port.c_str());
	}

	bool okay() const {
		return _okay;
	}

	std::string id() const {
		std::lock_guard lck(_model_lock);
		return _id;
	}

	bool err() const {
		return _err;
	}

	std::string model() const {
		std::lock_guard lck(_model_lock);
		return _model;
	}

	std::string version() const {
		std::lock_guard lck(_model_lock);
		return _version;
	}

	std::string hexId() const {
		std::lock_guard lck(_model_lock);
		return _hexId;
	}

	void write(const std::string &&msg) {
		std::unique_lock lck(write_queue_mutex);
		write_queue.push_back(std::move(msg));
		write_queue_cv.notify_all();
	}

};

