#include "ButtonMappingCode.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace librvl5mappingcode {
	// Given a character that represents an ASCII letter or number, get the character's hexadecimal value.
	// Returns -1 if the character is not one of: 0123456789ABCDEFabcdef
	int valueOfCharacter(char c) {
		if (c >= '0' && c <= '9') {
			return c - '0';
		}
		else if (c >= 'a' && c <= 'f') {
			return c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'F') {
			return c - 'A' + 10;
		}
		else {
			return -1;
		}
	}

	// Given a string, extracts and parses all hexadecimal characters (ignoring other characters).
	// Returns a vector of bytes.
	vector<uint8_t> parseHexString(string str) {
		vector<uint8_t> vec;
		bool tens = true;
		uint8_t current = 0;
		for (char &c : str) {
			int value = valueOfCharacter(c);
			if (value < 0) continue;

			if (tens) {
				current |= value << 4;
				tens = false;
			}
			else {
				current |= value;
				vec.push_back(current);
				current = 0;
				tens = true;
			}
		}
		return vec;
	}

	// Creates a new ButtonMappingCode object given a code's name and data.
	// Since the code is modified in place, you can give this contstructor a whole GCT file or just a single code.
	ButtonMappingCode::ButtonMappingCode(string name, const void* data, size_t dataLength) {
		this->_name = name;

		this->_dataStart = (uint16_t*)malloc(dataLength);
		this->_dataEnd = this->_dataStart + (dataLength / sizeof(uint16_t));

		const uint8_t* inData = (const uint8_t*)data;
		for (size_t i = 0; i < dataLength; i += 2) {
			this->_dataStart[i / 2] = inData[i] << 8 | inData[i + 1];
		}
	}

	// Creates a new ButtonMappingCode object given a code's name and data (in text format.)
	ButtonMappingCode::ButtonMappingCode(string name, string data) {
		this->_name = name;

		vector<uint8_t> parsed = parseHexString(data);
		this->_dataStart = (uint16_t*)malloc(parsed.size());
		this->_dataEnd = this->_dataStart + (parsed.size() / sizeof(uint16_t));

		const uint8_t* inData = parsed.data();
		for (size_t i = 0; i < parsed.size(); i += 2) {
			this->_dataStart[i / 2] = inData[i] << 8 | inData[i + 1];
		}
	}

	// Gets pointers to the data for all Classic Controller button mappings detected in this code.
	std::vector<ButtonMappingHeader*> ButtonMappingCode::getButtonMappings() {
		vector<ButtonMappingHeader*> vec;
		for (uint16_t* ptr = this->_dataStart; ptr < _dataEnd; ptr++) {
			bool match1 =
				ptr[0] == 0x70A4 &&
				ptr[2] == 0x2C04 &&
				ptr[3] == 0x0000 &&
				ptr[4] == 0x4182;
			if (match1) {
				vec.push_back((ButtonMappingHeader*)ptr);
			}
		}
		return vec;
	}

	// Gets the name of the code provided in the constructor.
	string ButtonMappingCode::getName() {
		return this->_name;
	}

	// Builds a multi-line string representation of this code.
	string ButtonMappingCode::toString() {
		string str = this->_name + '\n';
		char buf[5];
		int i = 0;
		for (uint16_t* ptr = this->_dataStart; ptr < this->_dataEnd; ptr++) {
			sprintf(buf, "%04X", *ptr);
			str += buf;
			switch (i++ % 4) {
			case 1:
				str += ' ';
				break;
			case 3:
				str += '\n';
				break;
			}
		}
		return str;
	}
}