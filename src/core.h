#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sodium.h>

#include "util.h"
#include "hashutil.h"
#include "fsutil.h"
#include "BitWriter.h"
#include "BitStream.h"

#include <stb_image_write.h>


/// @brief ChaCha20 PRNG (using libsodium)
class ChaCha20RNG {
private:
    uint8_t key[crypto_stream_chacha20_KEYBYTES];
    uint8_t nonce[crypto_stream_chacha20_NONCEBYTES];
    uint64_t counter;
    uint8_t buffer[64];
    size_t buffer_pos;

public:
    // --- UniformRandomBitGenerator requirements ---
    using result_type = uint32_t;
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT32_MAX; }
    // ----------------------------------------------

    ChaCha20RNG(uint32_t seed) : counter(0), buffer_pos(64) {
        memset(key, 0, crypto_stream_chacha20_KEYBYTES);
        memset(nonce, 0, crypto_stream_chacha20_NONCEBYTES);
        memset(buffer, 0, 64);

        uint8_t hash[32];
        uint8_t seed_bytes[4];
        seed_bytes[0] = (seed >> 0) & 0xFF;
        seed_bytes[1] = (seed >> 8) & 0xFF;
        seed_bytes[2] = (seed >> 16) & 0xFF;
        seed_bytes[3] = (seed >> 24) & 0xFF;

        crypto_hash_sha256(hash, seed_bytes, 4);
        memcpy(key, hash, crypto_stream_chacha20_KEYBYTES);
    }

    result_type operator()() {
        if (buffer_pos >= 64) {
            memset(buffer, 0, 64);
            memcpy(nonce, &counter,
                sizeof(counter) < crypto_stream_chacha20_NONCEBYTES
                    ? sizeof(counter)
                    : crypto_stream_chacha20_NONCEBYTES);
            crypto_stream_chacha20(buffer, 64, nonce, key);
            counter++;
            buffer_pos = 0;
        }

        result_type result;
        memcpy(&result, buffer + buffer_pos, sizeof(result_type)); // UB回避
        buffer_pos += sizeof(result_type);
        return result;
    }
};

void encode(Mode mode, const std::string& inputPng, const std::string& inputBin, const std::string& output) {
	std::string ext = fs::path(inputBin).extension().string().substr(1);
	if (ext.size() > 7) throw std::runtime_error("ext must len <= 7");

	// 埋め込みデータ
	BIN bin = loadBinFile(inputBin);

	// 画像読み込み
	int imageWidth, imageHeight;
	std::vector<Pixel> pixels = loadPNG(inputPng, imageWidth, imageHeight);
	uint32_t seed = hash256_px(pixels);

	// 容量チェック
	size_t usableBits = countUsableBits(pixels, mode);
	size_t requireBits = bin.size() * 8;

	std::cout << "Usable: " << formatBits(usableBits) << "\nRequire: " << formatBits(requireBits) << "\n";

	if (requireBits > usableBits)
		throw std::runtime_error("require > usable");

	// indices生成＆shuffle
	std::vector<int> indices(pixels.size());
	for (int i = 0; i < (int)pixels.size(); i++)
		indices[i] = i;

	ChaCha20RNG rng(seed);
	std::shuffle(indices.begin(), indices.end(), rng);

	// header
	Header hd{};
	hd.bitSize = requireBits;
	hd.crc = crc32(bin);
	memset(hd.ext, 0, sizeof(hd.ext));
	strncpy(hd.ext, ext.c_str(), 7);

	// header埋め込み
	BIN out;
	out.resize(sizeof(Header) + bin.size());

	memcpy(out.data(), &hd, sizeof(Header));
	memcpy(out.data() + sizeof(Header), bin.data(), bin.size());

	// bit読み取り用
	BitStream bs(out);

	// encode本体
	for (int idx : indices) {
		if (!bs.hasBits(1)) break;  // 1bit以上残っていれば続ける
		
		Pixel& p = pixels[idx];
		write(bs, p.r, mode);
		write(bs, p.g, mode);
		write(bs, p.b, mode);
	}

	// PNG出力
	if ( !stbi_write_png(output.c_str(),
					imageWidth, imageHeight, 3,// RGB
					pixels.data(), imageWidth * 3 // bytes
				)
		) throw std::runtime_error("faied to write png");
	std::cout << "saved to " << output << "\n";
}


// チャネル値からビットを抽出し、BitWriterに書き込む
void read(BitWriter& bw, uint8_t originalCh, uint8_t encodedCh, Mode mode) {
	if (canUseChannel(originalCh, mode)) {
		uint8_t bits;
		switch (mode) {
			case Mode::Bit1: 
				bits = readDelta1(originalCh, encodedCh);
				bw.writeBits(bits, 1);
				break;
			case Mode::Bit2:
				bits = readDelta2(originalCh, encodedCh);
				bw.writeBits(bits, 2);
				break;
		}
	}
}

void decode(Mode mode, const std::string& inputOriginalPng, const std::string& inputPng, const std::string& output) {
	// 元のPNG読み込み
	int w, h;
	std::vector<Pixel> originalPixels = loadPNG(inputOriginalPng, w, h);
	uint32_t seed = hash256_px(originalPixels);

	// エンコード済みPNG読み込み
	int _w, _h;
	std::vector<Pixel> encodedPixels = loadPNG(inputPng, _w, _h);

	// indices生成＆shuffle
	std::vector<int> indices(originalPixels.size());
	for (int i = 0; i < (int)originalPixels.size(); i++)
		indices[i] = i;

	ChaCha20RNG rng(seed);
	std::shuffle(indices.begin(), indices.end(), rng);

	// ビット抽出
	BitWriter bw;
	for (int idx : indices) {
		Pixel& original = originalPixels[idx];
		Pixel& encoded = encodedPixels[idx];

		read(bw, original.r, encoded.r, mode);
		read(bw, original.g, encoded.g, mode);
		read(bw, original.b, encoded.b, mode);
	}
	bw.flush();

	// HeaderとDataを分離
	const BIN& extracted = bw.get();
	if (extracted.size() < sizeof(Header))
		throw std::runtime_error("Extracted data too small for header");

	Header hd;
	memcpy(&hd, extracted.data(), sizeof(Header));

	// bitSizeをバイト数に変換
	size_t dataBytes = (hd.bitSize + 7) / 8;
	if (extracted.size() < sizeof(Header) + dataBytes)
		throw std::runtime_error("Extracted data size mismatch");

	BIN data(extracted.begin() + sizeof(Header),
	         extracted.begin() + sizeof(Header) + dataBytes);

	Header defa;

	// Header ver 確認
	if (hd.version != defa.version)
		throw std::runtime_error("Header version mismatch");

	// CRC32検証
	if (crc32(data) != hd.crc)
		throw std::runtime_error("CRC32 verification failed");

	// ファイル出力
	std::string outputFile = output + ".dmo." + hd.ext;
	std::ofstream outfile(outputFile, std::ios::binary);
	if (!outfile)
		throw std::runtime_error("Failed to open output file");

	outfile.write(reinterpret_cast<char*>(data.data()), data.size());
	outfile.close();

	std::cout << "Decoded " << formatBits(dataBytes*8) << "\n"
			  << "saved to " << outputFile << "\n";
}

void info(Mode mode, const std::string& path) {
	int imageWidth, imageHeight;
	std::vector<Pixel> pixels = loadPNG(path, imageWidth, imageHeight);
	size_t usableBits = countUsableBits(pixels, mode);
	std::cout << "Usable: " << formatBits(usableBits) << "\n";
}

void info(Mode mode, const std::string& imageA, const std::string& imageB) {
	// 元のPNG読み込み
	int w, h;
	std::vector<Pixel> originalPixels = loadPNG(imageA, w, h);
	uint32_t seed = hash256_px(originalPixels);

	// エンコード済みPNG読み込み
	int _w, _h;
	std::vector<Pixel> encodedPixels = loadPNG(imageB, _w, _h);

	// indices生成＆shuffle
	std::vector<int> indices(originalPixels.size());
	for (int i = 0; i < (int)originalPixels.size(); i++)
		indices[i] = i;

	ChaCha20RNG rng(seed);
	std::shuffle(indices.begin(), indices.end(), rng);

	// ビット抽出
	BitWriter bw;
	for (int idx : indices) {
		Pixel& original = originalPixels[idx];
		Pixel& encoded = encodedPixels[idx];

		read(bw, original.r, encoded.r, mode);
		read(bw, original.g, encoded.g, mode);
		read(bw, original.b, encoded.b, mode);
	}
	bw.flush();

	// HeaderとDataを分離
	const BIN& extracted = bw.get();
	if (extracted.size() < sizeof(Header))
		throw std::runtime_error("Extracted data too small for header");

	Header hd;
	memcpy(&hd, extracted.data(), sizeof(Header));

	std::cout << "ver:\t" << static_cast<int>(hd.version) << "\n"
			  << "size:\t" << formatBits(hd.bitSize) << "\n"
			  << "CRC32:\t" << hd.crc << "\n"
			  << "ext:\t" << hd.ext << "\n"
			  << "flag:\t" << hd.flags << "\n";
}