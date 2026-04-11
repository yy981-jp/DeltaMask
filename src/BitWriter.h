#pragma once
#include "def.h"


class BitWriter {
	BIN data;
	uint8_t current = 0;
	int bitIndex = 0;

	void writeBit(uint8_t b) {
		current <<= 1;
		current |= (b & 1);
		bitIndex++;

		if (bitIndex == 8) {
			data.push_back(current);
			current = 0;
			bitIndex = 0;
		}
	}

public:
	void writeBits(uint8_t v, int n) {
		for (int i = n - 1; i >= 0; i--) {
			writeBit((v >> i) & 1);
		}
	}

	void flush() {
		if (bitIndex != 0) {
			current <<= (8 - bitIndex);
			data.push_back(current);
			current = 0;
			bitIndex = 0;
		}
	}

	const BIN& get() const { return data; }
};
