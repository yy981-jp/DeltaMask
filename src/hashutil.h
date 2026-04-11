#pragma once
#include "def.h"


/// @brief fnv-1a hash
/// @param data 
/// @return 256bit hash
uint32_t hash256(const BIN& data) {
	uint32_t hash = 2166136261u;

	for (size_t i = 0; i < data.size(); i++) {
		hash ^= data[i];
		hash *= 16777619u;
	}

	return hash;
}

uint32_t crc32(const BIN& data) {
	static uint32_t table[256];
	static bool init = false;

	if (!init) {
		for (uint32_t i = 0; i < 256; i++) {
			uint32_t c = i;
			for (int j = 0; j < 8; j++) {
				if (c & 1)
					c = 0xEDB88320u ^ (c >> 1);
				else
					c >>= 1;
			}
			table[i] = c;
		}
		init = true;
	}

	uint32_t crc = 0xFFFFFFFFu;

	for (size_t i = 0; i < data.size(); i++) {
		crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
	}

	return crc ^ 0xFFFFFFFFu;
}

uint32_t fnv1a(const std::vector<Pixel>& pixels) {
	uint32_t hash = 2166136261u;

	for (auto& p : pixels) {
		hash ^= p.r;
		hash *= 16777619u;

		hash ^= p.g;
		hash *= 16777619u;

		hash ^= p.b;
		hash *= 16777619u;
	}

	return hash;
}

