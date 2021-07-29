#pragma once

#include <string>
#include <cstring>

class ParseBuffer;

struct ParseBufferStruct
{
	virtual void write(ParseBuffer& buffer) = 0;
	virtual void read(ParseBuffer& buffer) = 0;
};

class ParseBuffer
{
public:
	ParseBuffer(unsigned char* _data) : data(_data) {

	}

	void write_uint8(uint8_t v) {
		data[ptr] = v;
		ptr += 1;
	}

	void write_uint16(uint16_t v) {
		data[ptr] = (v >> 8) & 0xFF;
		data[ptr + 1] = (v & 0xFF);
		ptr += 2;
	}

	void write_uint32(uint32_t v) {
		data[ptr] = (v >> 24) & 0xFF;
		data[ptr + 1] = (v >> 16) & 0xFF;
		data[ptr + 2] = (v >> 8) & 0xFF;
		data[ptr + 3] = (v & 0xFF);
		ptr += 4;
	}

	void write_uint64(uint64_t v) {
		data[ptr] = (v >> 56) & 0xFF;
		data[ptr + 2] = (v >> 48) & 0xFF;
		data[ptr + 3] = (v >> 40) & 0xFF;
		data[ptr + 4] = (v >> 24) & 0xFF;
		data[ptr + 5] = (v >> 16) & 0xFF;
		data[ptr + 6] = (v >> 8) & 0xFF;
		data[ptr + 7] = (v & 0xFF);
		ptr += 8;
	}

	void write_float(float v) {
		memcpy(&data[ptr], &v, 4);
		ptr += 4;
	}

	void write_double(double v) {
		memcpy(&data[ptr], &v, 8);
		ptr += 8;
	}

	void write_string(const char* str, uint32_t strlen) {
		if (strlen == 0 || str == nullptr) { return; }

		for (uint32_t i = 0; i < strlen; i++) {
			data[ptr + i] = str[i];
		}

		ptr += strlen;
	}

	void write_padding(uint64_t size) {
		if (size > 0) {
			for (uint64_t i = 0; i < size; i++) {
				data[ptr + i] = 0;
			}
		}

		ptr += size;
	}

	uint8_t read_uint8() {
		ptr += 1;
		return data[ptr - 1];
	}

	uint16_t read_uint16() {
		ptr += 2;
		return ((data[ptr - 2] << 8) | (data[ptr - 1]));
	}

	uint32_t read_uint32() {
		ptr += 4;
		return ((data[ptr - 4] << 24) | (data[ptr - 3] << 16) | (data[ptr - 2] << 8) | (data[ptr - 1]));
	}

	uint64_t read_uint64() {
		ptr += 8;
		return ((data[ptr - 8] << 56) | (data[ptr - 7] << 48) | (data[ptr - 6] << 40) | (data[ptr - 5] << 32) | (data[ptr - 4] << 24) | (data[ptr - 3] << 16) | (data[ptr - 2] << 8) | (data[ptr - 1]));
	}

	float read_float() {
		float f;
		memcpy(&f, &data[ptr], 4);
		ptr += 4;
		return f;
	}

	double read_double() {
		double d;
		memcpy(&d, &data[ptr], 8);
		ptr += 8;
		return d;
	}

	std::string read_string(uint32_t strlen) {
		ptr += strlen;
		return std::string(&data[ptr - strlen], &data[ptr]);
	}

	void set_pointer(uint64_t _ptr) {
		if (_ptr >= 0) {
			ptr = _ptr;
		}
	}

	const uint64_t get_pointer() {
		return ptr;
	}
private:
	unsigned char* data{ nullptr };
	uint64_t ptr{ 0 };
};