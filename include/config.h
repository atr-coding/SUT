#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_map>

enum class CPARAMS : uint32_t {
	PARAMS = 0,
	INCLUDE_PATHS,
	LIB_PATHS,
	LIBS
};

typedef std::unordered_map<CPARAMS, std::string> param_map;

inline param_map load_config() {
	param_map params;

	std::string config_path = std::filesystem::current_path().string() + "/includes/config";
	std::ifstream config(config_path);
	if (config.is_open()) {
		std::cout << "Loading config.\n";
		std::string line;
		CPARAMS index = CPARAMS::PARAMS;
		while (std::getline(config, line)) {
			if (line == "params:") {
				index = CPARAMS::PARAMS;
			} else if (line == "include_paths:") {
				index = CPARAMS::INCLUDE_PATHS;
			} else if (line == "lib_paths:") {
				index = CPARAMS::LIB_PATHS;
			} else if (line == "libs:") {
				index = CPARAMS::LIBS;
			} else {
				if (line != "" && line.at(0) != '#') {
					switch (index) {
					case CPARAMS::PARAMS:
						params[index] += " " + line;
						break;
					case CPARAMS::INCLUDE_PATHS:
						params[index] += " -I\"" + line + "\"";
						break;
					case CPARAMS::LIB_PATHS:
						params[index] += " -L\"" + line + "\"";
						break;
					case CPARAMS::LIBS:
						params[index] += " -l" + line;
						break;
					}
				}
			}
		}
		config.close();
	}

	return params;
}