#pragma once
#include "stdafx.h"
#include "util.h"

namespace pt = boost::property_tree;

struct no_command_mapped_err { std::string line; };
struct command_mapping_load_err { std::string msg; };


inline std::string trim(const std::string &str) {
	auto first = str.find_first_not_of(' ');
	auto last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

struct cmd_with_condition {
	std::optional<std::tuple<std::string, int>> condition;
	std::string cmd;
};

typedef std::tuple<std::string, std::string> hardware_t;
typedef std::map<std::string, std::vector<cmd_with_condition>> cmd_map_t;
typedef std::vector<std::tuple<std::string, int>> dataref_t;

struct hardware_map_t {
	cmd_map_t cmd_map;
	dataref_t dataref_map;
};

class command_mapping {

private:
	std::map<hardware_t, hardware_map_t> mapping;

public:
	const cmd_map_t &cmd_map(const std::string &model, const std::string& id) const {
		try {
			return mapping.at(std::make_tuple(model, id)).cmd_map;
		} catch (std::out_of_range) {
			throw no_command_mapped_err{ model + id };
		}		
	}

	const dataref_t &dataref_map(const std::string &model, const std::string& id) const {
		try {
			return mapping.at(std::make_tuple(model, id)).dataref_map;
		} catch (std::out_of_range) {
			throw no_command_mapped_err{ model + id };
		}
	}

	void load(const std::string &cfg_path) {
		logMessage("command_mapping: loading %s", cfg_path.c_str());

		mapping = std::map<hardware_t, hardware_map_t>();

		pt::ptree tree;
		try {
			pt::read_ini(cfg_path, tree);
		} catch (pt::ini_parser_error ipe) {
			std::stringstream ss;
			ss << ipe.filename() << ":" << ipe.line() << ": " << ipe.message();
			throw command_mapping_load_err{ ss.str() };
		}

		for (const auto &[section, entries] : tree) {
			auto sep = section.find('#');
			if (sep == std::string::npos) {
				throw command_mapping_load_err{ "section header " + section + " is missing id" };
			}
			auto model = section.substr(0, sep);
			auto id_str = section.substr(sep + 1);

			auto hardware = std::make_tuple(model, id_str);
			auto &cmd_by_line = mapping[hardware].cmd_map;
			auto &dataref_by_pos = mapping[hardware].dataref_map;
			for (const auto &[key, value] : entries) {
				try {
					const auto line_or_pos = trim(key);
					const auto &cmd_or_dataref = trim(value.get_value<std::string>());
					if (line_or_pos.at(0) == '<') {
						size_t pos = std::stoi(line_or_pos.substr(1));
						auto hash = cmd_or_dataref.find('#');
						std::string dataref = cmd_or_dataref.substr(0, hash);
						std::string on_value = cmd_or_dataref.substr(hash + 1);
					
						// std::string on_value = cmd_or_dataref.substr(hash + 1);
						if (dataref_by_pos.size() <= pos)
							dataref_by_pos.resize(pos + 1);
						dataref_by_pos[pos] = { dataref, std::stoi(on_value) };
						//logMessage("command_mapping: %s#%s: %d <== %s == %s", model.c_str(), id_str.c_str(), pos, dataref.c_str(), on_value.c_str());
					}
					else {
						std::string line;
						cmd_with_condition cwc;
						cwc.cmd = cmd_or_dataref;

						auto at = line_or_pos.find('@');
						auto hash = line_or_pos.find('#');
						if (at != std::string::npos && hash != std::string::npos) {
							line = line_or_pos.substr(0, at);
							std::string dataref = line_or_pos.substr(at + 1, hash - at - 1);
							std::string on_value = line_or_pos.substr(hash + 1);
							cwc.condition = std::make_tuple(dataref, std::stoi(on_value));
							//logMessage("command_mapping: %s#%s: %s (for %s == %s) => %s", model.c_str(), id_str.c_str(),line.c_str(), dataref.c_str(), on_value.c_str(), cwc.cmd.c_str());
						} else {
							line = line_or_pos;	
							//logMessage("command_mapping: %s#%s: %s (unconditional) => %s", model.c_str(), id_str.c_str(),line.c_str(), cwc.cmd.c_str());
						}
						
						cmd_by_line[line].push_back(std::move(cwc));
					}
				} catch (...) {
					throw command_mapping_load_err{"entry for " + key + " is invalid"};
				}
			}
		}

		logMessage("command_mapping: loading completed");
	}

};

