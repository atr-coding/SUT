#pragma once

#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <filesystem>
#include <utility>
#include <sstream>
#include <fstream>

inline int64_t get_last_modified_time(const char* file) {
	struct stat result;
	return (stat(file, &result) == 0 ? result.st_mtime : -1);
}

inline void make_file_struct() {
	std::cout << "Creating file structure in " << std::filesystem::current_path() << '\n';

	std::string p = std::filesystem::current_path().string();
	std::filesystem::create_directory(p + "/tests");
	std::filesystem::create_directory(p + "/tests/bin");
	std::filesystem::create_directory(p + "/tests/include");
	std::filesystem::create_directory(p + "/tests/output");

	// Create framework.h
	std::ofstream framework(p + "/tests/include/framework.h", std::ios::trunc);
	if (framework.is_open()) {
		framework << "#pragma once\n#include <iostream>\n\
#define BEGIN_TEST() int main() { uint16_t count = 0;\n\
#define TEST(condition) if((condition) == false) { std::cout << \"f:\" << __LINE__ << ':' << #condition << ';'; } count++;\n\
#define TESTV(condition, v) if((condition) == false) { std::cout << \"f : \" << __LINE__ << ':' << #condition << \" (\" << v << \");\"; } count++;\n\
#define END_TEST() std::cout << \"c:\" << count << ';'; return 1; }";
		framework.close();
	} else {
		std::cout << "Unable to create framework file.\n";
	}

	// Create example config
	std::ofstream config(p + "/tests/include/config", std::ios::trunc);
	if (config.is_open()) {
		config << "params:\n#-w\n#-g\n\ninclude_paths:\n#/path/\n\nlib_paths:\n#/path/\n\nlibs:\n#ssl\n#crypto\n#ws2_32";
		config.close();
	}
}

inline std::string exec(const char* cmd) {

	char buffer[128];
	std::string result;
	FILE* pipe = popen(cmd, "r");

	if (!pipe) {
		std::cout << "Error: failed to open program: " << cmd << '\n';
	}

	try {
		while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
			result += buffer;
		}
	}
	catch (...) {
		pclose(pipe);
		return "";
	}

	pclose(pipe);

	return result;
}

inline std::vector<std::string> explode(std::string const& s, char delim) {
	std::vector<std::string> result;
	std::istringstream iss(s);

	for (std::string token; std::getline(iss, token, delim); ) {
		result.push_back(std::move(token));
	}

	return result;
}