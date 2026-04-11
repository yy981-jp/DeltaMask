#pragma once
#include "def.h"
#include <sodium.h>
#include <cstring>

/// @brief SHA-256 hash (using libsodium)
/// @param data 
/// @return 256bit hash
uint32_t hash256(const BIN& data) {
	uint8_t hash[crypto_hash_sha256_BYTES];
	crypto_hash_sha256(hash, data.data(), data.size());
	
	// Return first 32-bit of hash
	return (uint32_t)hash[0] << 24 | (uint32_t)hash[1] << 16 | 
	       (uint32_t)hash[2] << 8 | (uint32_t)hash[3];
}

/// @brief OBSOLETE - kept for compatibility, use hash256 instead
struct _SHA256_CTX_DEPRECATED {};

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

/// @brief SHA-256 hash for pixel data
/// @param pixels 
/// @return 256bit hash
uint32_t hash256_px(const std::vector<Pixel>& pixels) {
	BIN data;
	for (const auto& p : pixels) {
		data.push_back(p.r);
		data.push_back(p.g);
		data.push_back(p.b);
	}
	return hash256(data);
}

