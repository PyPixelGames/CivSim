#pragma once
#include "raylib.h"
#include <vector>

constexpr int chunkW = 32;
constexpr int chunkH = 32;

struct EditCell{
	int x;
	int y;
	char ch='?';
	Color c=PURPLE;
};

struct Chunk {
	int codepoints[chunkW * chunkH];
	Color colors[chunkW * chunkH];
	RenderTexture2D tex;
	std::vector<EditCell> cells;
};


