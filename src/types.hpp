#pragma once
#include "raylib.h"
#include <vector>
#include <unordered_map>
#include <cstdint>

constexpr int chunkW = 32;
constexpr int chunkH = 32;

struct Chunk {
	int codepoints[chunkW * chunkH];
	Color colors[chunkW * chunkH];
};


