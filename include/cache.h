#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "buffer.h"
#include "misc.h"

struct cache_data : public ParseBufferStruct {
	std::string file_name{ "" };
	uint64_t mod_time{ 0 };
	bool changed{ false };

	cache_data() {}
	cache_data(const std::string& _file_name, uint64_t _mod_time, bool _changed) : file_name(_file_name), mod_time(_mod_time), changed(_changed) {}

	void write(ParseBuffer& buffer) {
		buffer.write_uint8(file_name.size());
		buffer.write_string(file_name.c_str(), file_name.size());
		buffer.write_uint64(mod_time);
		buffer.write_padding(119 - file_name.size());
	}

	void read(ParseBuffer& buffer) {
		uint8_t fn_size = buffer.read_uint8(); // get the fn size [0-256]
		file_name = buffer.read_string(fn_size); // get the file name string [max size: 119] (119 + 1 byte for file name size + 8 bytes for mod time = 128 bytes = 8 records/KB)
		mod_time = buffer.read_uint64(); // get the mod time
		buffer.set_pointer(buffer.get_pointer() + 119 - fn_size); // skip over padding
	}
};

typedef std::vector<cache_data> cache_map;

inline void load_cache(cache_map& cache) {
	// Load in cache
	std::ifstream cache_file("cache", std::ios::binary);
	if (cache_file.is_open()) {
		std::cout << "Loading cache.\n";
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

				int64_t mod_time = get_last_modified_time((cd.file_name + ".cpp").c_str());
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
}

inline void update_cache(const cache_map& cache) {
	std::ofstream cache_file("cache", std::ios::binary | std::ios::trunc);
	if (cache_file.is_open()) {

		// Allocate space for new cache file
		unsigned char* buffer = new unsigned char[cache.size() * 128];

		// Parse all records
		ParseBuffer parse(buffer);
		for (auto c : cache) {
			c.write(parse);
		}

		// Write buffer to cache file
		cache_file.write((char*)buffer, cache.size() * 128);

		// Cleanup
		delete[] buffer;
		cache_file.close();
	}
}