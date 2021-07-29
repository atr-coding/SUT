#pragma once

#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <filesystem>
#include <utility>
#include <sstream>

inline int64_t get_last_modified_time(const char* file) {
	struct stat result;
	return (stat(file, &result) == 0 ? result.st_mtime : -1);
}

inline void make_file_struct() {
	std::string p = std::filesystem::current_path().string();
	std::filesystem::create_directory(p + "/tests");
	std::filesystem::create_directory(p + "/tests/bin");
	std::filesystem::create_directory(p + "/tests/include");
	std::filesystem::create_directory(p + "/tests/output");
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