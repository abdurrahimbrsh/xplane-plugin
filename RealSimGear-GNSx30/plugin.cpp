#include "stdafx.h"
#include "command_mapping.h"
#include "all_serial_watch.h"
#include "util.h"
#include "resource.h"
#include "gui/gui.h"
#include <regex>
#include <boost/algorithm/string.hpp>




namespace filesys = boost::filesystem;

const char *plugin_name = PRODUCT_NAME;
const char *plugin_sig = "realsimgear.gnsx30";
const char *plugin_desc = PRODUCT_DESC;
const char *mapping_filename = "CommandMapping.ini";
const char *device_mapping_filename = "DeviceMapping.ini";
const char *menu_title = PRODUCT_NAME;
const char *window_desc = PRODUCT_NAME;

const size_t XPluginStart_size = 256;
enum class menu : size_t { status };
enum class menu_id : int { status };
static void	menu_callback(void *inMenuRef, void *inItemRef);

bool g_loadCfgFromPlugin = false;
std::string detect_aircraft_path(void);

XPLMFlightLoopID flight_loop_id;
command_mapping cmd_map;
std::unique_ptr<all_serial_watch> watch;
std::deque<serial_msg> msg_queue;
std::mutex msg_queue_lock;

struct device_mapping {
	std::string device_id;
	std::string type;
	std::string port;
};
struct device_type
{
	std::regex device_pattern;
	std::vector<std::string> types;
};



std::vector < device_type > device_types{
										 {std::regex("RealSimGear-GN[A-Z]430.*"), {"CN2"}},
										 {std::regex("RealSimGear-GTN650.*"), {"CN2"}},
										 {std::regex("RealSimGear-GTN750.*"), {"CN1"}},
										 {std::regex("RealSimGear-GN[A-Z]530.*"), {"CN1", "CN2"}},
										 {std::regex("RealSimGear-GN[A-Z]430.*"), {"CN2"}},
										 {std::regex("RealSimGear-G1000[A-Z]FD.*"), {"PFD", "MFD"}},
										 {std::regex("RealSimGear-15\" MFD"), {"M15"}},
										 {std::regex("RealSimGear-GFC500"), {"GF5"}},
										 {std::regex("RealSimGear-GFC700"), {"GF7"}},
										 {std::regex("RealSimGear-GMA350"), {"GM3"}},
										 {std::regex("RealSimGear-GCU"), {"GCU"}},
										 {std::regex("RealSimGear-TBM Panel"), {"TP1"}},
										 {std::regex("RealSimGear-MCU"), {"MCU"}},
										 {std::regex("RealSimGear-G5 ASI"), {"G5A"}},
										 {std::regex("RealSimGear-G5 HSI"), {"G5H"}} };

std::vector < std::string > get_device_types(const std::string& device)
{
	for (auto& device_type : device_types)
	{
		if (std::regex_search(device, device_type.device_pattern))
			return device_type.types;
	}
	return {};
}

using device_mappings = std::vector<device_mapping>;

device_mappings g_device_maps;

device_mapping & get_device_map(const std::string& device_id)
{
	auto it = std::find_if(g_device_maps.begin(), g_device_maps.end(), [&device_id](auto& e) { return e.device_id == device_id; });
	if (it != g_device_maps.end())
		return *it;

	device_mapping device_map = { device_id, "", "" };
	g_device_maps.push_back(device_map);
	return g_device_maps.back();
}

std::string get_type(const std::string& device_id)
{
	auto it = std::find_if(g_device_maps.begin(), g_device_maps.end(), [&device_id](auto& e) { return e.device_id == device_id; });
	return it != g_device_maps.end() ? it->type : "";
}

device_mappings read_device_mappings(const std::string& file_name)
{
	std::ifstream file(file_name);
	std::string line;

	if (!file.is_open())
		return {};

	device_mappings tmp_device_maps;

	log("Load device mappting from %s", file_name.c_str());

	while (std::getline(file, line))
	{
		device_mapping device_map;
		auto tokens = tokenize(line, ",");
		if (tokens.size() != 3)
		{
			log("Wrong DM file format 1 - %s", line.c_str());
			continue;
		}

		std::string t0, t1, t2;
		t0 = tokens[0];
		t1 = tokens[1];
		t2 = tokens[2];
		boost::trim(tokens[0]);
		boost::trim(tokens[1]);
		boost::trim(tokens[2]);

		if (tokens[0].size() < 1 || tokens[1].size() < 1 || tokens[1].size() < 2)
		{
			log("Wrong DM file format 2 - %s", line.c_str());
			continue;
		}

		//logMessage("dm:%s,%s,%s", tokens[0].c_str(), tokens[1].c_str(), tokens[2].c_str());
		tmp_device_maps.push_back({ tokens[0], tokens[1], tokens[2] });
	}

	if (file.is_open())
		file.close();

	return tmp_device_maps;
}

std::string get_global_cfg_path()
{
	auto plugin_path = get_plugin_path();
	auto plugin_dir = path::remove_filename(plugin_path);
#if defined(__APPLE__) && !defined(XPLM300) 
	auto cfg_path = plugin_dir + path::seperator();
#else
	auto cfg_path = plugin_dir + path::seperator() + ".." + path::seperator();
#endif
	return cfg_path;
}

void write_device_mappings()
{
	/*If loading DM.ini or CM.ini from an aircraft folder, any updates to DM.ini should be saved to same folder*/
	std::string file_name = "";
	if (!g_loadCfgFromPlugin)
	{
		// Get aircraft path
		std::string aircraft_path = detect_aircraft_path();
		// Make path shorter
		auto aircraft_dir = path::remove_filename(aircraft_path);
		// Check to see if path contains a config file
		auto cfg_path = aircraft_dir + path::seperator();

		file_name = cfg_path + device_mapping_filename;
		logMessage("wrtie cfg to aircraft folder");
	}
	else
	{
		file_name = get_global_cfg_path() + device_mapping_filename;
		logMessage("wrtie cfg to plugin folder");
	}

	std::ofstream file;
	file.open(file_name, std::ofstream::out | std::ofstream::trunc);
	if (!file.is_open())
	{
		log("cannot open file %s for writing", file_name.c_str());
		return;
	}

	for (const auto& map : g_device_maps)
	{
		if (map.device_id.size() < 1 || map.type.size() < 1 || map.port.size() < 1)
		{
			log("DM cfg,invalid mapping, skip");
			continue;
		}

		file << map.device_id << ',' << map.type << ',' << map.port << std::endl;
	}

	if (file.is_open())
		file.close();
}


struct line_commandref_t {
	std::optional<std::tuple<XPLMDataRef, int>> condition;
	XPLMCommandRef commandref;
	std::string command;
};

struct hardware_status {
	std::string hardware;
	std::string port;
	std::string version;
	std::string last_line;
	std::string last_cmd;
	std::string last_out_line;
	std::map<std::string, std::vector<line_commandref_t>> line_commandref;
	std::optional<std::vector<bool>> pin_state;
	std::vector<std::tuple<XPLMDataRef, int>> pin_dataref_and_on_value;

};
std::map<std::string, hardware_status> status_by_id;

struct status_line_widget {
	std::shared_ptr<gui::label> id;
	std::shared_ptr<gui::label> hardware;
	std::shared_ptr<gui::label> port;
	std::shared_ptr<gui::label> version;
	std::shared_ptr<gui::label> last_line;
	std::shared_ptr<gui::label> last_cmd;
	std::shared_ptr<gui::label> last_out_line;
};

std::unique_ptr<gui::window> status_window;
std::shared_ptr<gui::button> connect_button;
std::shared_ptr<gui::button> disconnect_button;
std::vector<status_line_widget> status_line_labels;

std::string limit_string(const std::string &str, size_t max_length) {
	if (str.size() <= max_length)
		return str;
	else {
		auto start = str.length() - max_length + 3;
		if (start >= str.length())
			start = 0;
		return "..." + str.substr(start);
	}
}

void update_status_gui() {
	const std::string empty = "-----";
	if (watch != nullptr) {
		// connection enabled 

		for (size_t id = 1; id < status_line_labels.size(); id++) {
			const auto &w = status_line_labels[id];
			auto id_str = std::to_string(id) + ".";
			w.id->set_text(std::to_string(id));

			if (id <= status_by_id.size()) {
				auto it = status_by_id.begin();
				std::advance(it, id - 1);
				const auto& s = it->second;
				w.hardware->set_text(s.hardware);
				w.port->set_text(limit_string(s.port, 27));
				w.version->set_text(limit_string(s.version, 27));
				w.last_line->set_text(limit_string(s.last_line, 27));
				w.last_cmd->set_text(limit_string(s.last_cmd, 27));
				if (s.last_out_line.size() > 1)
					w.last_out_line->set_text(s.last_out_line.substr(1));
				else
					w.last_out_line->set_text(empty);
			}
			else {
				w.hardware->set_text(empty);
				w.port->set_text(empty);
				w.version->set_text(empty);
				w.last_line->set_text(empty);
				w.last_cmd->set_text(empty);
				w.last_out_line->set_text(empty);
			}
		}

		connect_button->set_enabled(false);
		disconnect_button->set_enabled(true);
	}
	else {
		// connection disabled (disconnect clicked)
		for (size_t id = 1; id < status_line_labels.size(); id++) {
			const auto & w = status_line_labels[id];
			w.id->set_text("");
			w.hardware->set_text("");
			w.port->set_text("");
			w.version->set_text("");
			w.last_line->set_text("");
			w.last_cmd->set_text("");
			w.last_out_line->set_text("");
			if (id == 3) {
				w.last_line->set_text("disconnected");
			}
		}
		connect_button->set_enabled(true);
		disconnect_button->set_enabled(false);
	}
}

void handle_msg(const serial_msg &msg) {
	std::lock_guard lck(msg_queue_lock);
	msg_queue.push_back(msg);
}

void process_connected(const serial_msg &msg) {
	log("%s with id %s has connected at port %s with version %s,hex id:%s",
		msg.model.c_str(), msg.id.c_str(), msg.port.c_str(), msg.version.c_str(),msg.hex_id.c_str());

	auto &status = status_by_id[msg.id];
	status.hardware = msg.model;
	status.port = msg.port;
	status.version = msg.version;
	status.last_line = "-----";
	status.last_cmd = "-----";

	try {
		auto& device_map = get_device_map(msg.id);
		if (device_map.type.empty())
		{
			auto types = get_device_types(msg.model);
			if (types.empty())
			{
				log("Types are not defined for the device %s.", msg.model.c_str());
				return;
			}
			device_map.type = types[0];
			device_map.port = msg.port;
		}

		auto type = device_map.type;
		if (type.empty())
			log("Unable to find type in device mappings for device id %s", msg.id.c_str());

		auto cmd_by_line = cmd_map.cmd_map(msg.model, type);
		for (const auto &[line, cwcs] : cmd_by_line) {
			log("Resolving commands for line: %s", line.c_str());
			for (const auto &cwc : cwcs) {
				line_commandref_t lcr;

				if (cwc.condition) {
					const auto &[dataref_name, cond_value] = *cwc.condition;
					log("Resolving X-Plane DataRef for condition: %s", dataref_name.c_str());
					XPLMDataRef dataref = XPLMFindDataRef(dataref_name.c_str());
					if (dataref != nullptr) {
						lcr.condition = std::make_tuple(dataref, cond_value);
					}
					else {
						log("X-Plane DataRef for condition not found: %s", dataref_name.c_str());
						continue;
					}
				}

				// First check to see if this is a simple command or a dataref set command
				if (auto delim_pos = cwc.cmd.find('|'); delim_pos != std::string::npos) { // look for "|", if found, its a dataref set command
					log("Resolving X-Plane DataRef: %s", cwc.cmd.c_str());
					auto cmd_dataref = cwc.cmd.substr(0, delim_pos);  // break apart to get to dataref
					XPLMDataRef cmdref = XPLMFindDataRef(cmd_dataref.c_str());
					if (cmdref != nullptr) {
						lcr.commandref = cmdref;
						lcr.command = cwc.cmd;
					}
					else {
						log("X-Plane DataRef not found: %s", cwc.cmd.c_str());
						continue;
					}
				}
				else {
					log("Resolving X-Plane CommandRef: %s", cwc.cmd.c_str());
					XPLMCommandRef cmdref = XPLMFindCommand(cwc.cmd.c_str());
					if (cmdref != nullptr) {
						lcr.commandref = cmdref;
						lcr.command = cwc.cmd;
					}
					else {
						log("X-Plane CommandRef not found: %s", cwc.cmd.c_str());
						continue;
					}
				}
				status.line_commandref[line].push_back(std::move(lcr));
			}
		}

		auto dataref_map = cmd_map.dataref_map(msg.model, type);
		for (const auto &[name, on_value] : dataref_map) {
			log("Resolving X-Plane DataRef: %s", name.c_str());
			XPLMDataRef dataref = XPLMFindDataRef(name.c_str());
			if (dataref != nullptr) {
				status.pin_dataref_and_on_value.push_back({ dataref, on_value });
			}
			else {
				status.pin_dataref_and_on_value.push_back({ nullptr, 0 });
				log("X-Plane DataRef not found: %s", name.c_str());
			}
		}
	}
	catch (no_command_mapped_err) {
		log("No mapping for device %s with id %s", msg.model.c_str(), msg.id.c_str());
	}

	update_status_gui();
}

void process_disconnected(const serial_msg &msg) {
	log("%s with id %s has disconnected from port %s.",
		msg.model.c_str(), msg.id.c_str(), msg.port.c_str());

	status_by_id.erase(msg.id);
	update_status_gui();
}

enum class action { begin, end, once, set_dataref };

void process_command(const serial_msg &msg) {
	auto &status = status_by_id[msg.id];
	status.last_line = msg.line;

	std::string cmd_part;
	std::ostringstream tmp_cmd_part;
	action act = action::once;
	bool pot_cmd = false;
	float pot_value = 0;
	std::string pot_value_st;
	if (auto eq_pos = msg.line.find('='); eq_pos != std::string::npos) {
		cmd_part = msg.line.substr(0, eq_pos);
		auto val_part = msg.line.substr(eq_pos);
		if (auto pot_pos = cmd_part.find("POT"); pot_pos != std::string::npos) {
			pot_cmd = true;
			pot_value_st = val_part;
			pot_value = std::stof(pot_value_st.erase(0, 1));
		};
		if (val_part == "=1") {
			act = action::begin;
		}
		else if (val_part == "=0") {
			act = action::end;
		}
	}
	else {
		cmd_part = msg.line;
	}

	try {
		std::string cmd;
		XPLMDataRef cmd_dataref = nullptr;
		XPLMCommandRef cmd_ref = nullptr;
		std::int8_t dataref_value = 0;
		std::float_t old_value = 0;
		std::float_t dataref_valuef;
		bool cmd_found = false;
		bool run_cmd = true;
		bool num_float = false;

		const auto &lcrs = status.line_commandref.at(cmd_part);
		for (const auto &lcr : lcrs) {
			if (lcr.condition) {
				const auto &[dataref, cond_value] = *lcr.condition;
				int value = 0;
				if (dataref != nullptr)
					value = XPLMGetDatai(dataref);
				if (value == cond_value) {
					run_cmd = true;
				}
				else {
					run_cmd = false;
				}
			}

			if (run_cmd) {
				// determine whether the cmd_part is a cmd or a dataref setting
				if (auto delim_pos = lcr.command.find('|'); delim_pos != std::string::npos) { // look for "|", if found, its a dataref setting
					// Only act on the press, not the release
					if (act == action::begin || act == action::once) {
						// Check for calculation need
						if (auto calc_pos = lcr.command.find('+'); calc_pos != std::string::npos) { // look for "+", if found, its a calculation
							// Get the value and calculate the result
							old_value = (XPLMGetDataf(lcr.commandref));
							dataref_valuef = old_value + std::stoi(lcr.command.substr(delim_pos + 2, std::string::npos));
							num_float = true;
						}
						else if (auto calc2_pos = lcr.command.find('-'); calc2_pos != std::string::npos) {  // look for "-", if found, its a calculation
							// Get the value and calculate the result
							old_value = (XPLMGetDataf(lcr.commandref));
							dataref_valuef = old_value - std::stoi(lcr.command.substr(delim_pos + 2, std::string::npos));
							num_float = true;
						}
						else {
							if (pot_cmd) {
								num_float = true;
								if (std::stoi(lcr.command.substr(delim_pos + 1, std::string::npos)) == 0) {
									dataref_valuef = pot_value / 1024;
								}
								else {
									dataref_valuef = 1 - (pot_value / 1024);
								}
							}
							else {
								dataref_value = std::stoi(lcr.command.substr(delim_pos + 1, std::string::npos));
							}
						}
						cmd = lcr.command;
						cmd_dataref = lcr.commandref;
						cmd_found = true;
						act = action::set_dataref;
						break;
					}
				}
				else {
					// if not, then simply set to run the cmd
					cmd = lcr.command;
					cmd_ref = lcr.commandref;
					cmd_found = true;
					break;
				}
			}
		}

		if (cmd_found) {
			switch (act) {
			case action::begin:
				logMessage("X-Plane command begin: %s", cmd.c_str());
				XPLMCommandBegin(cmd_ref);
				status.last_cmd = cmd + " (begin)";
				break;
			case action::end:
				logMessage("X-Plane command end:   %s", cmd.c_str());
				XPLMCommandEnd(cmd_ref);
				status.last_cmd = cmd + " (end)";
				break;
			case action::once:
				logMessage("X-Plane command once:  %s", cmd.c_str());
				XPLMCommandOnce(cmd_ref);
				status.last_cmd = cmd;
				break;
			case action::set_dataref:
				if (num_float) {
					logMessage("X-Plane set dataref:  %s , value %f", cmd.c_str(), dataref_valuef);
					XPLMSetDataf(cmd_dataref, dataref_valuef);
				}
				else {
					logMessage("X-Plane set dataref:  %s , value %i", cmd.c_str(), dataref_value);
					XPLMSetDatai(cmd_dataref, dataref_value);
				}
				status.last_cmd = cmd;
				break;
			}
		}
		else {
			logMessage("No conditional command match for line: %s", cmd_part.c_str());
		}

	}
	catch (std::out_of_range) {
		log("Command from %s with id %s not mapped: %s",
			msg.model.c_str(), msg.id.c_str(), msg.line.c_str());
		status.last_cmd = "(not mapped)";
	}
	update_status_gui();
}

void process_msg() {
	std::lock_guard lck(msg_queue_lock);
	while (!msg_queue.empty()) {
		const auto &msg = msg_queue.front();
		switch (msg.type) {
		case serial_msg_type::connected:
			process_connected(msg);
			break;
		case serial_msg_type::disconnected:
			process_disconnected(msg);
			break;
		case serial_msg_type::command:
			process_command(msg);
			break;
		}
		msg_queue.pop_front();
	}
}

void transmit_state() {
	for (auto &[id, status] : status_by_id) {
		if (status.pin_dataref_and_on_value.empty())
			continue;

		std::vector<bool> pin_state;
		for (const auto &[dataref, on_value] : status.pin_dataref_and_on_value) {
			int value = 0;
			if (dataref != nullptr)
				value = XPLMGetDatai(dataref);
			pin_state.push_back(value == on_value);
		}

		if (!status.pin_state || *status.pin_state != pin_state) {
			std::stringstream ss;
			ss << "<";
			for (bool v : pin_state) {
				if (v)
					ss << "1";
				else
					ss << "0";
			}
			ss << "\n";
			watch->write(id, ss.str());

			status.pin_state = std::move(pin_state);
			status.last_out_line = ss.str();
			update_status_gui();
		}
	}
}

float flight_loop_callback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop,
	int inCounter, void *inRefcon) {
	//logMessage("Flight loop callback started");
	process_msg();
	transmit_state();
	//logMessage("Flight loop callback completed");
	return -1;
}

void start_watch() {
	if (watch == nullptr) {
		std::lock_guard lck(msg_queue_lock);
		msg_queue.clear();

		status_by_id.clear();
		std::set < std::string > ports;
		watch = std::make_unique<all_serial_watch>(&handle_msg, ports);
		update_status_gui();
	}
}

void stop_watch() {
	if (watch != nullptr) {
		watch.reset();
		write_device_mappings();
		update_status_gui();
	}
}

void menu_callback(void *inMenuRef, void *inItemRef) {
	switch (static_cast<menu>((size_t)inItemRef)) {
	case menu::status:
		logMessage("Showing main widget");
		status_window->set_visible(true);
		break;
	}
}


std::string detect_aircraft_path(void) {
	char filename[256];
	char path[512];
	XPLMGetNthAircraftModel(XPLM_USER_AIRCRAFT, filename, path);
	std::string aircraft_path(path);
	return aircraft_path;
}

bool loadCfgFiles(const std::string& cmd_file, const std::string& device_file)
{
	try {
		log("Loading command mappings from %s", cmd_file.c_str());
		cmd_map.load(cmd_file);
	}
	catch (command_mapping_load_err err) {
		log("Loading command mappings failed: %s", err.msg.c_str());
		return false;
	}

	g_device_maps = read_device_mappings(device_file);
	return true;
}


void load_cfg_files()
{
	bool aircraft_found = true;
	// Get aircraft path
	std::string aircraft_path = detect_aircraft_path();
	// Make path shorter
	auto aircraft_dir = path::remove_filename(aircraft_path);
	// Check to see if path contains a config file
	auto cfg_path = aircraft_dir + path::seperator();

	auto cmd_mappings_path = cfg_path + mapping_filename;

	g_loadCfgFromPlugin = false;

	aircraft_found = loadCfgFiles(cmd_mappings_path, cfg_path + device_mapping_filename);

	if (!aircraft_found) {
		logMessage("Load cfg from plugin folder");
		g_loadCfgFromPlugin = true; 
		//auto plugin_path = get_plugin_path();
		//auto plugin_dir = path::remove_filename(plugin_path);
		auto cfg_path = get_global_cfg_path();
		auto cmd_mappings_path = cfg_path + mapping_filename;
		loadCfgFiles(cmd_mappings_path, cfg_path + device_mapping_filename);
	}
}

void button_handler(std::shared_ptr<gui::button> clicked_button) {
	if (clicked_button == connect_button)
	{
		load_cfg_files();
		start_watch();
	}
	if (clicked_button == disconnect_button)
	{
		stop_watch();
		write_device_mappings();
	}
}


float RSGDeferredFLCB(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
	load_cfg_files();
	start_watch();
	log("Plugin enabled");
	return 0;
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
	log("%s %s", PRODUCT_NAME, PRODUCT_VERSION);

	// plugin metadata
	strcpy_s(outName, XPluginStart_size, plugin_name);
	strcpy_s(outSig, XPluginStart_size, plugin_sig);
	strcpy_s(outDesc, XPluginStart_size, plugin_desc);

	// plugin menu
	int menu_item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), menu_title, 0, 1);
	XPLMMenuID menu = XPLMCreateMenu(menu_title, XPLMFindPluginsMenu(), menu_item,
		menu_callback, (void *)menu::status);
	XPLMAppendMenuItem(menu, "Connected Hardware", (void *)menu_id::status, 1);

	// flight loop
	XPLMCreateFlightLoop_t fl;
	fl.structSize = sizeof(fl);
	fl.phase = xplm_FlightLoop_Phase_BeforeFlightModel;
	fl.callbackFunc = &flight_loop_callback;
	fl.refcon = nullptr;
	flight_loop_id = XPLMCreateFlightLoop(&fl);
	XPLMScheduleFlightLoop(flight_loop_id, -1, false);

	// plugin window
	int height = 150, width = 900;
	auto title = std::string(PRODUCT_NAME) + " " + PRODUCT_VERSION;
	status_window = std::make_unique<gui::window>(title, 20, 100, width, height);

	// line
	auto line = std::make_shared<gui::line>(0, 15, width, 0);
	status_window->add_widget(line);

	// black background box
	auto bbox = std::make_shared<gui::box>(0, 20, width, 100);
	status_window->add_widget(bbox);

	// create status lines
	const int n_status_lines = 8;
	int y = 0;
	for (int i = 0; i < n_status_lines; i++) {
		status_line_widget l;
		gui::color col(1.0f, 1.0f, 1.0f);
		int x = 5;
		l.id = std::make_shared<gui::label>("", x, y, col); x += 20;
		l.hardware = std::make_shared<gui::label>("Hardware", x, y, col); x += 160;
		l.port = std::make_shared<gui::label>("Port", x, y, col); x += 100;
		l.version = std::make_shared<gui::label>("Version", x, y, col); x += 100;
		l.last_line = std::make_shared<gui::label>("Message", x, y, col); x += 205;
		l.last_cmd = std::make_shared<gui::label>("Command", x, y, col); x += 205;
		l.last_out_line = std::make_shared<gui::label>("LEDs", x, y, col); x += 100;
		status_window->add_widget(l.id);
		status_window->add_widget(l.hardware);
		status_window->add_widget(l.port);
		status_window->add_widget(l.version);
		status_window->add_widget(l.last_line);
		status_window->add_widget(l.last_cmd);
		status_window->add_widget(l.last_out_line);
		status_line_labels.push_back(std::move(l));
		if (i == 0)
			y += 20;
		else
			y += 14;
	}

	// disconnect button
	int button_width = 100, button_height = 25;
	y = height - button_height;
	int x = width;
	disconnect_button =
		std::make_shared<gui::button>("Disconnect", button_handler,
			x - button_width, y, button_width, button_height);
	status_window->add_widget(std::static_pointer_cast<gui::widget>(disconnect_button));
	x -= button_width + 8;

	// disconnect button
	connect_button =
		std::make_shared<gui::button>("Connect", button_handler,
			x - button_width, y, button_width, button_height);
	status_window->add_widget(std::static_pointer_cast<gui::widget>(connect_button));

	//status_window->set_visible(true);
	log("Plugin started");
	return 1;
}

PLUGIN_API void	XPluginStop() {
	log("Stopping plugin");
	XPLMDestroyFlightLoop(flight_loop_id);
	connect_button.reset();
	disconnect_button.reset();
	status_line_labels.clear();
	status_window.reset();
	log("Plugin stopped");
}

PLUGIN_API int XPluginEnable() {
	log("Enabling plugin");
	XPLMRegisterFlightLoopCallback(RSGDeferredFLCB, -1, NULL);
	//try {
	//	// Get aircraft path
	//	detect_aircraft_path();
	//	log("Path is %s", aircraft_path);

	//	// Check to see if path contains a config file

	//	auto plugin_path = get_plugin_path();
	//	auto plugin_dir = path::remove_filename(plugin_path);
	//	#if defined(__APPLE__) && !defined(XPLM300) 
	//	auto cfg_path =
	//		plugin_dir + path::seperator() + mapping_filename;
	//	#else
	//	auto cfg_path =
	//		plugin_dir + path::seperator() + ".." + path::seperator() + mapping_filename;
	//	#endif
	//	log("Loading command mappings from %s", cfg_path.c_str());
	//	cmd_map.load(cfg_path);
	//} catch (command_mapping_load_err err) {
	//	log("Loading command mappings failed: %s", err.msg.c_str());
	//}
	//start_watch();

	//log("Plugin enabled");
	return 1;
}

PLUGIN_API void XPluginDisable() {
	log("Disabling plugin");
	stop_watch();
	log("Plugin disabled");
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, int inMessage, void *inParam) {

}


/*Save log to file, for debugging purpose only*/
void logMessage(const char* fmt, ...)
{
#if 1
#pragma warning (disable : 4996)
	va_list argptr;
	char msg[1024];
	va_start(argptr, fmt);
	vsprintf_s(msg, 1024, fmt, argptr);
	va_end(argptr);

	std::string logPath = get_global_cfg_path() + "plugin.log";

	FILE* pFile = fopen(logPath.c_str(), "a");
	fprintf(pFile, "%s\n", msg);
	fclose(pFile);
#endif
}
