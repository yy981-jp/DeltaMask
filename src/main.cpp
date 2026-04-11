#include <string>
#include <iostream>
#include <cstring>
#include <random>

#include "util.h"
#include "hashutil.h"
#include "fsutil.h"
#include "BitWriter.h"
#include "BitStream.h"

#include <stb_image_write.h>


void encode(Mode mode, std::string inputPng = "input.png", std::string inputBin = "input.txt", const std::string output = "output.png") {
	std::string ext = fs::path(inputBin).extension().string().substr(1);
	if (ext.size() > 7) throw std::runtime_error("ext must len <= 7");

	// 埋め込みデータ
	BIN bin = loadBinFile(inputBin);

	// 画像読み込み
	int imageWidth, imageHeight;
	std::vector<Pixel> pixels = loadPNG(inputPng, imageWidth, imageHeight);
	uint32_t seed = fnv1a(pixels);

	// 容量チェック
	size_t usableBits = countUsableBits(pixels, mode);
	size_t requireBits = bin.size() * 8;

	std::cout << "Usable: " << usableBits << "\nRequire: " << requireBits << "\n";

	if (requireBits > usableBits)
		throw std::runtime_error("require > usable");

	// indices生成＆shuffle
	std::vector<int> indices(pixels.size());
	for (int i = 0; i < (int)pixels.size(); i++)
		indices[i] = i;

	std::mt19937 rng(seed);
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
		Pixel& p = pixels[idx];

		if (!bs.hasBits(bitsPerChannel(mode) * 3))
			break;

        write(bs, p.r, mode);
        write(bs, p.b, mode);
        write(bs, p.g, mode);
	}

	// PNG出力
	if ( stbi_write_png(output.c_str(),
					imageWidth, imageHeight, 3,// RGB
					pixels.data(), imageWidth * 3 * 8 // bytes
				)
		) throw std::runtime_error("faied to write png");
}

int main() {
    encode(Mode::Bit1);
}