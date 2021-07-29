#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "buffer.h"
#include "misc.h"

struct cache_data : public ParseBufferStruct {
	std::string file_name{ "" }; // source file names without .cpp extension
	uint64_t mod_time{ 0 }; // time of last modification, given by get_last_modified_time in misc
	bool changed{ false };

	cache_data() {}
	cache_data(const std::string& _file_name, uint64_t _mod_time, bool _changed) : file_name(_file_name), mod_time(_mod_time), changed(_changed) {}

	void write(ParseBuffer& buffer) {
		buffer.write_uint64(mod_time);
		buffer.write_uint8(file_name.size());
		buffer.write_string(file_name.c_str(), file_name.size());
	}

	void read(ParseBuffer& buffer) {
		mod_time = buffer.read_uint64(); // get the mod time
		uint8_t fn_size = buffer.read_uint8(); // get the fn size [0-256]
		file_name = buffer.read_string(fn_size); // get the file name string [max size: 119] (119 + 1 byte for file name size + 8 bytes for mod time = 128 bytes = 8 records/KB)
	}
};

typedef std::vector<cache_data> cache_map;

inline cache_map load_cache() {
	// Load in cache
	std::string current_path = std::filesystem::current_path().string();
	std::ifstream cache_file(current_path + "/tests/cache", std::ios::binary);
	cache_map cache;
	if (cache_file.is_open()) {
		//std::cout << "Loading cache.\n";

		// Get cache file size
		cache_file.seekg(0, std::ios::end);
		uint64_t file_size = cache_file.tellg();
		cache_file.seekg(0, std::ios::beg);

		if (file_size >= 10) { // 10 is the smallest possible size of a record
			unsigned char* data = new unsigned char[file_size];
			cache_file.read((char*)data, file_size);

			ParseBuffer buffer(data);

			uint64_t ptr = buffer.get_pointer();
			while (ptr < file_size) {

				cache_data cd;
				cd.read(buffer);

				std::stringstream ss;
				ss << current_path << "/tests/" << cd.file_name << ".cpp";
				int64_t mod_time = get_last_modified_time(ss.str().c_str());

				if (mod_time != -1) {
					if (mod_time > cd.mod_time) {
						cd.changed = true;
						cd.mod_time = mod_time;
					}
					cache.push_back(cd);
				}
				ptr = buffer.get_pointer();
			}

			delete[] data;
		}
		cache_file.close();
	}
	return cache;
}

inline void update_cache(const cache_map& cache) {
	std::fstream cache_file(std::filesystem::current_path().string() + "/tests/cache", std::ios::out | std::ios::binary | std::ios::trunc);
	if (cache_file.is_open()) {

		uint64_t buffer_size{ cache.size() * 9 }; // calculate size of the "mod" time variables (8 bytes each) + 1 byte for the file name length

		// Add the length of the file names
		for (const auto& c : cache) {
			buffer_size += c.file_name.size();
		}

		// Allocate space for new cache file
		unsigned char* buffer = new unsigned char[buffer_size];

		// Parse all records
		ParseBuffer parse(buffer);
		for (auto c : cache) {
			c.write(parse);
		}

		// Write buffer to cache file
		cache_file.write((char*)buffer, buffer_size);

		// Cleanup
		delete[] buffer;
		cache_file.close();
	}
}