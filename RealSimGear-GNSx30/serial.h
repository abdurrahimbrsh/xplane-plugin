#pragma once
#include "stdafx.h"
#include "util.h"

#include <stdio.h>

struct serial_error {
	std::string port;
	std::string msg;

	serial_error(std::string port, std::string msg) : port(port), msg(msg) { }
};

struct serial_timeout {};



#if defined(__APPLE__) || defined(__linux__)
inline void serial_signal_handler(int signal) { }
#endif



class serial {

private:
	bool opened = false;

	#if defined(WIN32)
	HANDLE fd;
	std::mutex io_state_mutex;
	bool io_active;
	DWORD io_thread_id;

	void open() {
		auto unc_port = "\\\\.\\" + port;
		fd = CreateFile(unc_port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE) {
			throw serial_error(port, "CreateFile failed on " + unc_port + ": " + get_last_error_as_string());
		}

		COMMCONFIG cc;
		DWORD cc_size = sizeof(cc);
		if (!GetCommConfig(fd, &cc, &cc_size)) {
			throw serial_error(port, "GetCommConfig failed: " + get_last_error_as_string());
		}
		cc.dcb.BaudRate = baud;
		cc.dcb.ByteSize = 8;
		cc.dcb.Parity = NOPARITY;
		cc.dcb.StopBits = ONESTOPBIT;
		cc.dcb.fRtsControl = RTS_CONTROL_DISABLE;
		cc.dcb.fDtrControl = DTR_CONTROL_DISABLE;
		cc.dcb.fOutxCtsFlow = FALSE;
		cc.dcb.fOutxDsrFlow = FALSE;
		cc.dcb.fOutX = FALSE;
		cc.dcb.fInX = FALSE;
		cc.dcb.fDsrSensitivity = FALSE;
		if (!SetCommConfig(fd, &cc, cc_size)) {
			throw serial_error(port, "SetCommConfig failed: " + get_last_error_as_string());
		}
		
		COMMTIMEOUTS ct;
		if (!GetCommTimeouts(fd, &ct)) {
			throw serial_error(port, "GetCommTimeouts failed: " + get_last_error_as_string());
		}
		ct.ReadIntervalTimeout = timeout;
		ct.ReadTotalTimeoutConstant = timeout;
		ct.ReadTotalTimeoutMultiplier = 0;
		ct.WriteTotalTimeoutConstant = timeout;
		ct.WriteTotalTimeoutMultiplier = 0;
		if (!SetCommTimeouts(fd, &ct)) {
			throw serial_error(port, "SetCommTimeouts failed: " + get_last_error_as_string());
		}

		logMessage("%s: port opened", port.c_str());
	}

	void close() {
		CloseHandle(fd);
		logMessage("%s: port closed", port.c_str());
	}

	char read_char() {
		char buf[1];
		DWORD n_read;

		{
			std::unique_lock lck(io_state_mutex);
			io_active = true;
			io_thread_id = GetCurrentThreadId();
		}
		if (!ReadFile(fd, buf, 1, &n_read, NULL)) {
			std::unique_lock lck(io_state_mutex);
			io_active = false;
			throw serial_error(port, "ReadFile failed: " + get_last_error_as_string());
		}
		{
			std::unique_lock lck(io_state_mutex);
			io_active = false;
		}

		if (n_read == 0)
			throw serial_timeout();
		return buf[0];
	}

	#define BUFFER_SIZE 1024
	static std::set<std::string> get_all_ports_impl() {
		std::set<std::string> ports;

		HKEY hKey;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 
			             0, KEY_READ, &hKey) != ERROR_SUCCESS) {
			logMessage("RegOpenKeyEx failed: %s", get_last_error_as_string().c_str());
			return ports;
		}

		DWORD idx = 0;
		while (true) {
			char value_name[BUFFER_SIZE];
			char value_data[BUFFER_SIZE];
			DWORD value_name_len = BUFFER_SIZE;
			DWORD value_data_len = BUFFER_SIZE;
			DWORD type;
			LONG res = RegEnumValue(hKey, idx, value_name, &value_name_len, NULL,
									&type, (LPBYTE)value_data, &value_data_len);
			if (res == ERROR_SUCCESS) {
				if (type == REG_SZ) {
					//logMessage("Enumerated port: %s = %s", value_name, value_data);
					std::string port(value_data);
					ports.insert(port);
				}
				idx++;
			} else if (res == ERROR_NO_MORE_ITEMS) {
				break;
			} else {
				throw serial_error("", "RegEnumValue failed: " + get_last_error_as_string());
			}
		}

		if (RegCloseKey(hKey) != ERROR_SUCCESS) {
			throw serial_error("", "RegCloseKey failed: " + get_last_error_as_string());
		}

		return ports;
	}

	void cancel_io_impl() {
		std::unique_lock lck(io_state_mutex);
		if (!io_active) {
			logMessage("%s: no I/O to cancel", port.c_str());
			return;
		}

		HANDLE hThread = OpenThread(THREAD_TERMINATE, false, io_thread_id);
		if (hThread == NULL)
			throw serial_error(port, "OpenThread failed: " + get_last_error_as_string());
		if (!CancelSynchronousIo(hThread)) {
			logMessage("CancelSynchronousIo failed: %s", get_last_error_as_string().c_str());
		}
		CloseHandle(hThread);
	}
	#endif

	#if defined(__APPLE__) || defined(__linux__)
	
	int fd;
	std::mutex io_state_mutex;
	bool io_active;
	pthread_t io_thread_id;

	void open() {
		fd = ::open(port.c_str(), O_RDWR | O_NOCTTY);
		if (fd == -1) {
			throw serial_error(port, "open failed: " + get_last_error_as_string());
		}
		if (ioctl(fd, TIOCEXCL) == -1) {
			throw serial_error(port, "ioctl TIOCEXCL failed: " + get_last_error_as_string());
		}

		struct termios settings;
		if (tcgetattr(fd, &settings) == -1) {
			throw serial_error(port, "tcgetattr failed: " + get_last_error_as_string());
		}
		cfmakeraw(&settings);
		settings.c_cc[VMIN] = 0;
		settings.c_cc[VTIME] = timeout / 100;
		if (baud == 115200)
			cfsetspeed(&settings, B115200);
		else
			throw serial_error(port, "only supported baud rate is 115200");
		settings.c_cflag &= ~(CSIZE | PARENB);
		settings.c_cflag |= CS8;
		if (tcsetattr(fd, TCSANOW | TCSAFLUSH, &settings) == -1) {
			throw serial_error(port, "tcsetattr failed: " + get_last_error_as_string());
		}		

		// register signal handler
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler = &serial_signal_handler;
		if (sigaction(SIGHUP, &act, nullptr) == -1) {
			throw serial_error(port, "sigaction failed: " + get_last_error_as_string());
		}
		
		logMessage("%s: port opened", port.c_str());
	}

	void close() {
		// unregister signal handler
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler = SIG_DFL;
		if (sigaction(SIGHUP, &act, nullptr) == -1) {
			throw serial_error(port, "sigaction failed: " + get_last_error_as_string());
		}		

		::close(fd);
		logMessage("%s: port closed", port.c_str());
	}

	char read_char() {
		{
			std::unique_lock<std::mutex> lck(io_state_mutex);
			io_active = true;
			io_thread_id = pthread_self();
		}

		char buf[1];
		ssize_t n_read = read(fd, buf, 1);
		if (n_read == -1) {
			std::unique_lock<std::mutex> lck(io_state_mutex);
			io_active = false;
			throw serial_error(port, "read failed: " + get_last_error_as_string());
		}

		{
			std::unique_lock<std::mutex> lck(io_state_mutex);
			io_active = false;
		}

		if (n_read == 0)
			throw serial_timeout();
		return buf[0];
	}

	void cancel_io_impl() {
		std::unique_lock<std::mutex> lck(io_state_mutex);
		if (!io_active) {
			logMessage("%s: no I/O to cancel", port.c_str());
			return;
		}
		#if defined(__linux__)
		pthread_kill(io_thread_id, SIGHUP);
		#elif defined(__APPLE__)
		// X-Plane on MacOS crashes when signals are used.
		logMessage("%s: waiting for I/O to complete (MacOS workaround)", port.c_str());
		#endif
	}	

	#endif

	#if defined(__linux__)

	inline static const char *serial_enum_path = "/dev/serial/by-id";
	static std::set<std::string> get_all_ports_impl() {		
		std::set<std::string> ports;				

		DIR *dir = opendir(serial_enum_path);
		if (dir == NULL) {
			auto err_str = get_last_error_as_string();
			logMessage("opendir %s failed: %s", serial_enum_path, err_str.c_str());
			return ports;
		}
		
		while (true) {
			struct dirent *ent = readdir(dir);
			if (ent == NULL)
				break;
			std::string port_filename(ent->d_name); 
			if (port_filename == "." || port_filename == "..")
				continue;
			std::string port = std::string(serial_enum_path) + "/" + port_filename;
			//logMessage("Enumerated port: %s", port.c_str());
			ports.insert(port);
		}

		if (closedir(dir) == -1) {
			throw serial_error("", "closedir failed: " + get_last_error_as_string());
		}
		return ports;
	}

	#elif defined (__APPLE__)

	#define DEVICE_BUFFER_SIZE 1024
	static std::set<std::string> get_all_ports_impl() {		
		std::set<std::string> ports;				

		// get MACH port
		mach_port_t master_port;
		kern_return_t result = IOMasterPort(MACH_PORT_NULL, &master_port);
		if (result != KERN_SUCCESS)	{
			throw serial_error("", "IOMasterPort failed: " + std::to_string(result));
		}

		// Serial devices are instances of class IOSerialBSDClient.
		CFMutableDictionaryRef classes_to_match = IOServiceMatching(kIOSerialBSDServiceValue);
		if (classes_to_match == NULL) {
			throw serial_error("", "IOServiceMatching returned a NULL dictionary");
		}

		// set RSR232 type to match
		CFDictionarySetValue(classes_to_match, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));

		// get iterator over matched devices
		io_iterator_t rs232_iter;
		result = IOServiceGetMatchingServices(master_port, classes_to_match, &rs232_iter);
		if (result != KERN_SUCCESS)	{
			throw serial_error("", "IOServiceGetMatchingServices failed: " + std::to_string(result));
		}

		// iterate over all matched devices
		io_object_t rs232_service;
		while ((rs232_service = IOIteratorNext(rs232_iter)) != 0)  {
			CFTypeRef device_file_path_cf = 
				IORegistryEntryCreateCFProperty(rs232_service, CFSTR(kIOCalloutDeviceKey),
												kCFAllocatorDefault, 0);
			if (device_file_path_cf) {
				// convert to C string
				char device_file_path[DEVICE_BUFFER_SIZE];
				Boolean result = CFStringGetCString((CFStringRef)device_file_path_cf, device_file_path,
						     					    DEVICE_BUFFER_SIZE, kCFStringEncodingASCII);
				CFRelease(device_file_path_cf);
				if (result)	{
					ports.insert(std::string(device_file_path));
					logMessage("Enumerated port: %s", device_file_path);
				}
			}
		 	IOObjectRelease(rs232_service);
		}
		IOObjectRelease(rs232_iter);

		return ports;
	}	

	#endif
	


public:

	const std::string port;
	const int baud;
	const int timeout;
	const int max_length;

	serial(std::string port, int baud, int timeout, int max_length) 
		: port(port), baud(baud), timeout(timeout), max_length(max_length) {
		open();
		opened = true;
	}

	~serial() {
		if (opened)
			close();
	}

	std::string read_line() {
		std::stringstream ss;
		int length = 0;
		while (length < max_length) {
			char c = read_char();
			if (c == '\n' || c == '\r')
				return ss.str();
			ss << c;
			length++;
		}
		throw serial_timeout();
	}

	static std::set<std::string> get_all_ports() {
		return get_all_ports_impl();
	}

	#if defined(WIN32)
	void write(const std::string &s) {
		DWORD n_written;
		if (!WriteFile(fd, s.c_str(), (DWORD)s.size(), &n_written, NULL)) 
			throw serial_error(port, "WriteFile failed: " + get_last_error_as_string());
		if (!FlushFileBuffers(fd))
			throw serial_error(port, "FlushFileBuffers failed: " + get_last_error_as_string());
		if (n_written != s.size())
			throw serial_timeout();
	}
	#endif

	#if defined(__APPLE__) || defined(__linux__)	

	void write(const std::string &s) {
		ssize_t n_written = ::write(fd, s.c_str(), s.size());		
		if (n_written == -1) 
			throw serial_error(port, "write failed: " + get_last_error_as_string());
		if (n_written != s.size())
			throw serial_timeout();
	}

	#endif

	void cancel_io() {
		logMessage("%s: cancelling I/O", port.c_str());
		cancel_io_impl();
		logMessage("%s: I/O cancelled", port.c_str());
	}

};

