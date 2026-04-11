#pragma once
#include <cstdint>
#include <vector>


#pragma pack(push, 1)
struct Header {
	uint32_t magic = 0x5944544D; // YDTM (= yy981 DeltaMask)
	uint8_t version = 1;

    uint8_t flags = 0;

	uint32_t bitSize;
	// uint32_t byteSize;

	uint32_t crc; // 埋め込みデータのCRC32

	char ext[8] = {};
};
#pragma pack(pop)


using BIN = std::vector<uint8_t>;

enum class Mode {
	Bit1, Bit2
};

struct Pixel {
	uint8_t r, g, b;
};

constexpr int bitsPerChannel(Mode mode) {
	switch (mode) {
		case Mode::Bit1: return 1;
		case Mode::Bit2: return 2;
	}
	return 0;
}
