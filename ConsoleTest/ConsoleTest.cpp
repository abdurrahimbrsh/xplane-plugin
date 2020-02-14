#include <string>
#include <iostream>
#include <cstdio>

#include "serial.h"
#include "serial_watch.h"
#include "all_serial_watch.h"
#include "command_mapping.h"


void test_serial_enum()
{
	std::cout << "Enumerating serial ports:" << std::endl;
	auto all_ports = serial::get_all_ports();
	for (std::string port : all_ports)
		std::cout << port << std::endl;
	std::cout << std::endl;
}


void test_serial()
{
	std::string port("COM4");
	int baud = 9600;
	int timeout = 10000;
	int max_length = 1024;
	
	serial s(port, baud, timeout, max_length);

	while (true) {
		try {
			std::string line = s.read_line();
			std::cout << "RECEIVED: " << line << std::endl;
		} catch (serial_timeout) {
			std::cout << "TIMEOUT" << std::endl;
		} catch (serial_error) {
			std::cout << "Serial error" << std::endl;
			return;
		}
	}
}


void prn_msg(const serial_msg &msg) {
	std::cout << "message: " << msg.str() << std::endl;
}

void test_serial_watch() {
	std::list<serial_watch> watches;
	for (auto port : serial::get_all_ports()) {
		watches.emplace_back(port, &prn_msg);
	}

	std::cout << "Press ENTER to stop." << std::endl;
	std::string dummy;
	std::getline(std::cin, dummy);
	std::cout << "Quiting." << std::endl;
}


void test_all_serial_watch() {
	all_serial_watch asw(prn_msg);

	std::cout << "Press ENTER to stop." << std::endl;
	std::string dummy;
	std::getline(std::cin, dummy);
	std::cout << "Quiting." << std::endl;
}


void test_command_mapping() {
	command_mapping cmd_map;
	cmd_map.load("../CommandMapping.ini");

	std::cout << "RealSimGear-GNS530#1: sim/GPS/g430n1_nav_ff=" <<
		cmd_map.resolve("RealSimGear-GNS530", 1, "sim/GPS/g430n1_nav_ff") << std::endl;
	std::cout << "RealSimGear-GNS530#1: sim/GPS/g430n1_nav_ff=" <<
		cmd_map.resolve("RealSimGear-GNS530", 1, "sim/GPS/g430n1_com_ff") << std::endl;
	std::cout << "RealSimGear-GNS430#2: sim/GPS/g430n2_proc=" <<
		cmd_map.resolve("RealSimGear-GNS430", 2, "sim/GPS/g430n2_proc") << std::endl;
}


int main() {
	test_command_mapping();
	test_all_serial_watch();
	//test_serial_watch();
	//test_serial_enum();
	//test_serial();
    return 0;
}

