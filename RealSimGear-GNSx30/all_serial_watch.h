#pragma once
#include "stdafx.h"
#include "util.h"
#include "serial.h"
#include "serial_watch.h"


const std::chrono::seconds scan_interval(5);


class all_serial_watch {

private:
	// msg_received must be before port_watch to have correct destruction order.
	// (a serial_watch sends a message on destruction that is dispatched using
	//  msg_received and thus it must still be available when the serial_watches
	//  are destructed)
	std::function<void(const serial_msg &)> msg_received;
	std::map<std::string, std::shared_ptr<serial_watch> > port_watch;
	std::mutex port_watch_mutex;
	std::thread watch_thread;
	std::atomic<bool> _stop = false;
	std::mutex _stop_mutex;
	std::condition_variable _stop_cv;
	std::set < std::string > _port_list;

	void scan() {
		std::map<std::string, std::shared_ptr<serial_watch> > new_port_watch;
		{
			std::unique_lock lck(port_watch_mutex);
			new_port_watch = port_watch;
		}

		//trace("all_serial_watch: scanning ports");
		auto avail_ports = serial::get_all_ports();

		// remove disconnected ports
		std::list<std::string> to_remove;
		for (auto const &[port, watch] : new_port_watch) {
			bool use_port = _port_list.empty() || _port_list.find(port) != _port_list.end();
			if (avail_ports.count(port) == 0 || watch->err() || !use_port)
				to_remove.push_back(port);
		}
		for (auto port : to_remove) {
			logMessage("all_serial_watch: removing disconnected/errored port %s", port.c_str());
			new_port_watch.erase(port);
		}

		// add available ports
		for (auto port : avail_ports) {
			/*if (port.find("2") != std::string::npos)
			{
				//logMessage("Except %s for debugging purpose!!!",port.c_str());
				continue;
			}*/

			if (new_port_watch.count(port) == 0) {
				logMessage("all_serial_watch: adding new port %s", port.c_str());
				new_port_watch[port] = 
					std::make_shared<serial_watch>(port, [this](const auto &msg) { msg_from_port(msg); });
			}
		}

		{
			std::unique_lock lck(port_watch_mutex);
			port_watch = std::move(new_port_watch);
		}
	}

	void watch() {
		std::unique_lock<std::mutex> lck(_stop_mutex);
		while (!_stop) {
			scan();
			_stop_cv.wait_for(lck, scan_interval);
		}
	}

	void msg_from_port(const serial_msg &msg) {
		logMessage("all_serial_watch: msg: %s", msg.str().c_str());
		msg_received(msg);
	}

public:
	all_serial_watch(std::function<void(const serial_msg &)> msg_received, const std::set<std::string> & port_list) 
		: msg_received(msg_received), _port_list(port_list) {
		watch_thread = std::thread([=] {this->watch(); });
	}

	~all_serial_watch() {
		logMessage("all_serial_watch: stopping");

		{
			std::unique_lock lck(_stop_mutex);
			_stop = true;
		}
		_stop_cv.notify_all();		
		watch_thread.join();

		logMessage("all_serial_watch: stopped");
	}

	void write(const std::string& id, const std::string &&msg) {
		std::unique_lock lck(port_watch_mutex);
		for (auto &[port, watch] : port_watch) {
			(void)port;
			if (watch->id() == id && !watch->err()) {
				log("all_serial_watch: msg to %s on %s: %s", id.c_str(), port.c_str(), msg.c_str());
				watch->write(std::move(msg));
				return;
			}
		}
		log("all_serial_watch: cannot deliver msg to unknown id %s: %s", id.c_str(), msg.c_str());
	}

};

