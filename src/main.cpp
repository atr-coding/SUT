#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <utility>
#include <fstream>

#include "../include/buffer.h"
#include "../include/misc.h"
#include "../include/cache.h"
#include "../include/config.h"

namespace fs = std::filesystem;

const std::string html_begin = "<!DOCTYPE html><html><head><style>\
body{ padding: 0px; margin: 0px; font-family: Arial, Helvetica, sans-serif; background-color: rgb(37, 37, 37); color: white; }\
#container{ padding: 0px; margin: 10px auto; width: 550px; }\
table{ width: 550px; border-collapse:collapse; }\
.test{ padding: 10px; width: 500px; vertical-align: middle; border: 1px solid white; }\
.icon{ padding: 10px; width: 50px; vertical-align: middle; text-align: center; border: 1px solid white; color: rgb(165, 255, 153); }\
.bad{ color: rgb(255, 122, 122); }\
</style><link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/icon?family=Material+Icons\"></head><body><div id=\"container\"><table>";
const std::string html_end = "</table></div></body></html>";

auto getTestFiles() {
	std::vector<fs::path> results;
	if (fs::is_directory("tests")) {
		auto path = fs::path("tests");
		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_regular_file() && entry.path().extension().string() == ".cpp") {
				results.push_back(entry.path());
			}
		}
		return results;
	} else {
		std::cout << "\"tests\" folder not found.\n";
		std::cout << "You can re-run the program with the -m flag to create the necessary file strcuture.\n";
	}
	return results;
}

auto getTestPrograms() {
	std::vector<fs::path> results;
	if (fs::is_directory("bin")) {
		auto path = fs::path("tests/bin");
		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_regular_file() && entry.path().has_extension() == false) {
				results.push_back(entry.path());
			}
		}
		return results;
	} else {
		std::cout << "\"bin\" folder not found.\n";
		std::cout << "You can re-run the program with the -m flag to create the necessary file strcuture.\n";
	}
	return results;
}

bool compile(param_map& pmap, fs::path file) {
	std::cout << "Compiling " << file.filename().string() << "...";
	std::ostringstream ss;
	ss << "g++ " << pmap[CPARAMS::PARAMS] << pmap[CPARAMS::INCLUDE_PATHS] << " \"" << file.string() << "\"" << pmap[CPARAMS::LIB_PATHS] << pmap[CPARAMS::LIBS] << " -o \"tests/bin/" << file.filename().stem().string() << "\"";
	if (system(ss.str().c_str()) == 0) {
		std::cout << "complete.\n";
		return true;
	} else {
		std::cout << "failed.\n";
		return false;
	}
}

void cleanup() {
	std::cout << "Cleaning up...\n";
	auto files = getTestFiles();
	auto programs = getTestPrograms();

	std::vector<fs::path> output;

	for (const auto& p : programs) {

		bool found{ false };
		for (const auto& f : files) {
			if (f.stem() == p.stem()) {
				found = true;
			}
		}

		if (!found) {
			output.push_back(p);
		}
	}

	std::cout << "Found " << output.size() << " tests that need to be cleaned up:\n";
	for (auto o : output) {
		std::cout << "tests/bin/" << o.string() << '\n';
	}

	std::cout << "Remove these files? [y/n]: ";
	std::string input;
	std::cin >> input;
	std::cout << '\n';
	if (input == "y") {
		std::cout << "Removing...";
		for (auto o : output) {
			std::cout << "tests/bin/" << o.string() << '\n';
			fs::remove("tests/bin/" + o.string());
		}
	}
}

/*
Flags:
m 	- create file structure
c 	- compile new and changed tests
ca 	- compile all test
r 	- run tests
ra	- run all tests
-cleanup - cleans up all test programs that no longer have a corresponding source file
*/

struct FailCase {
	std::string test;
	uint16_t line_number{ 0 };
};

struct ParsedRunResults {
	uint16_t count{ 0 };
	std::vector<FailCase> fail_cases;
};

ParsedRunResults parse_run_results(const std::string& exec_ret) {
	ParsedRunResults results;
	auto fails = explode(exec_ret, ';');
	for (auto fail : fails) {
		auto f = explode(fail, ':');
		char type = f.at(0).at(0);
		if (type == 'f') {
			if (f.size() >= 3) {
				results.fail_cases.push_back({ f.at(2), (uint16_t)std::stoi(f.at(1)) });
			}
		} else if (type == 'c') {
			if (f.size() >= 2) {
				results.count = std::stoi(f.at(1));
			}
		}
	}
	return results;
}

int main(int argc, char* argv[]) {

	// Load and set config parameters
	auto cparams = load_config();

	// Handle arguments
	bool c{ false };
	bool ca{ false };
	bool r{ false };
	bool ra{ false };
	bool o{ false };

	if (argc >= 2) {
		for (int i = 0; i < argc; i++) {
			std::string arg = argv[i];
			if (arg == "-m") {
				std::cout << "Creating file structure in current directory...\n";
				make_file_struct();
			} else if (arg == "-c") {
				c = true;
				ca = false;
			} else if (arg == "-ca") {
				ca = true;
				c = false;
			} else if (arg == "-r") {
				r = true;
				ra = false;
			} else if (arg == "-ra") {
				ra = true;
				r = false;
			} else if (arg == "-cleanup") {
				cleanup();
			} else if (arg == "-o") {
				o = true;
			}
		}
	} else {
		std::cout << "Commands: \n";
		std::cout << "-m\t\t- create file structure\n";
		std::cout << "-c\t\t- compile new/changed tests\n";
		std::cout << "-ca\t\t- compile all tests\n";
		std::cout << "-r\t\t- run new/changed tests\n";
		std::cout << "-ra\t\t- run all tests\n";
		std::cout << "-o\t\t- output result of tests to console and not the web\n";
		std::cout << "-cleanup\t\t- cleanup tests that have been compiled, but whose source file has been delete.\n";
	}


	if (c) {
	} else if (ca) {
		auto files = getTestFiles();
		for (auto f : files) {
			compile(cparams, f);
		}
	}

	if (r) {

	} else if (ra) {
		std::ofstream html("tests/output/index.html", std::ios::trunc);
		if (html.is_open()) {
			html << html_begin;
			auto programs = getTestPrograms();
			for (auto p : programs) {
				auto fails = parse_run_results(exec(p.string().c_str()));
				std::stringstream ss;
				ss << "<tr><td class=\"test\">" << p.filename().stem().string();
				ss << "&emsp;&emsp;(" << (fails.count - fails.fail_cases.size()) << '/' << fails.count << ")</td><td class=\"icon\"><i class=\"material-icons\">check_circle</i></td></tr>";
				html << ss.str();
				ss.str("");
				for (auto fail : fails.fail_cases) {
					if (o) {
						std::cout << "Test: " << fail.test << " failed on line " << fail.line_number << " of tests/" << p.filename().stem().string() << ".cpp\n";
					}
					ss << "<tr><td class=\"test\">&emsp;&emsp;" << fail.test << " failed on line " << fail.line_number;
					ss << "</td><td class=\"icon bad\"><i class=\"material-icons\">check_circle</i></td></tr>";
					html << ss.str();
					ss.str("");
				}
			}
			html << html_end;
			html.close();
		} else {
			std::cout << "tests/output/index.html failed to open.\n";
		}
	}
}