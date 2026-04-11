#pragma once
#include "def.h"


class BitStream {
    std::vector<uint8_t> data;
    size_t bitPos = 0; // ← これだけ！

public:
    BitStream(const std::vector<uint8_t>& d)
        : data(d) {}

    bool hasBits(size_t len) const {
        return (bitPos + len) <= data.size() * 8;
    }

    uint8_t getBit() {
        if (bitPos >= data.size() * 8) {
            throw std::out_of_range("BitStream overflow");
        }

        size_t byteIndex = bitPos / 8;
        int bitIndex = bitPos % 8;

        uint8_t b = (data[byteIndex] >> (7 - bitIndex)) & 1;

        bitPos++;
        return b;
    }

    uint32_t getBits(int n) {
        uint32_t v = 0;
        for (int i = 0; i < n; i++) {
            v = (v << 1) | getBit();
        }
        return v;
    }
};
