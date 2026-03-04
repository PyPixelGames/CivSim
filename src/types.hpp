#pragma once
#include "raylib.h"
#include <vector>

constexpr int chunkW = 32;
constexpr int chunkH = 32;

struct Chunk {
	int codepoints[chunkW * chunkH];
	Color colors[chunkW * chunkH];
};


