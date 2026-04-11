#pragma once

#include "def.h"

#include <filesystem>
#include <fstream>
#include <stb_image.h>

namespace fs = std::filesystem;


BIN loadBinFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error("loadBinFile: failed to open binFile");

    std::streamsize size = file.tellg();
    file.seekg(0);

    BIN buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    // 実際に読んだデータ数にvectorを一致させる
    buffer.resize(file.gcount());

    return buffer;
}

std::vector<Pixel> loadPNG(const std::string& path, int& w, int& h) {
	int ch;
    std::vector<Pixel> out;

	unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 3); // RGB固定

	if (!data) throw std::runtime_error("Failed to load image: " + path);

	out.resize(w * h);

	for (int i = 0; i < w * h; i++) {
		out[i].r = data[i * 3 + 0];
		out[i].g = data[i * 3 + 1];
		out[i].b = data[i * 3 + 2];
	}

	stbi_image_free(data);

    return out;
}
