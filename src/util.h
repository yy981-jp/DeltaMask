#pragma once
#include "def.h"
#include "BitStream.h"


bool canUseChannel(uint8_t A, Mode mode) {
	switch (mode) {
		case Mode::Bit1: return (A >= 1 && A <= 254);
		case Mode::Bit2: return (A >= 2 && A <= 253);
	}
	return false;
}

size_t countUsableBits(const std::vector<Pixel>& pixels, Mode mode) {
	size_t count = 0;

	for (auto& p : pixels) {
		if (canUseChannel(p.b, mode)) count++;
		if (canUseChannel(p.g, mode)) count++;
		if (canUseChannel(p.r, mode)) count++;
	}

	return count * bitsPerChannel(mode);
}

inline void applyDelta1(uint8_t& v, bool bit) {
	v += (bit ? 1 : -1);
}

void applyDelta2(uint8_t& v, uint8_t bits) {
	int delta =
		(bits == 0) ? -2 :
		(bits == 1) ? -1 :
		(bits == 2) ? +1 : +2;

	v += delta;
}

// Bit1モードで差分からビットを復元
uint8_t readDelta1(uint8_t original, uint8_t encoded) {
	int delta = (int)encoded - (int)original;
	return (delta > 0) ? 1u : 0u;
}

// Bit2モードで差分からビットを復元
uint8_t readDelta2(uint8_t original, uint8_t encoded) {
	int delta = (int)encoded - (int)original;
	switch (delta) {
		case -2: return 0u;
		case -1: return 1u;
		case  1: return 2u;
		case  2: return 3u;
		default: return 0u;
	}
}

void write(BitStream& bs, uint8_t& ch, Mode mode) {
	if (canUseChannel(ch, mode)) {
		uint8_t b = bs.getBits(bitsPerChannel(mode));
		switch (mode) {
			case Mode::Bit1: applyDelta1(ch, b); break;
			case Mode::Bit2: applyDelta2(ch, b); break;
		}
	}
}
