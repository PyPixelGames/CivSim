#pragma once
#include "types.hpp"
#include <unordered_map>
#include <cstdint>
#include <string>
#include "loadFile.hpp"

inline int64_t getKey(int cx, int cy) {
	return (int64_t(cx) << 32) | int64_t(uint32_t(cy));
}

Chunk makeChunk() {
	Chunk c;
	for (int i = 0; i < chunkW * chunkH; i++) {
		c.codepoints[i] = '`';
		c.colors[i] = Color{52, 52, 52, 255};
	}
	return c;
}

std::pair<int, int> resize(int cellSize, Font font){
	int glyphIndex = GetGlyphIndex(font, 'A');
	int advance = font.glyphs[glyphIndex].advanceX;

	float scale = (float)cellSize / font.baseSize;

	int gap = 4; //increase this value to decrease the gap size

	int cellW = (int)((advance - font.glyphs[GetGlyphIndex(font,'A')].offsetX)*scale)-gap;
	int cellH = (int)((font.baseSize - font.glyphs[GetGlyphIndex(font,'A')].offsetY) * scale)-gap;

	return {cellW, cellH};
}

void populateChunks(std::unordered_map<int64_t, Chunk>& world,int chunksX, int chunksY){
	for (int cy = 0; cy < chunksY; cy++) {
		for (int cx = 0; cx < chunksX; cx++) {
			int64_t key = getKey(cx, cy);
			if (world.find(key) == world.end()) {
				world[key] = makeChunk();
			}
		}
	}
}

void changeTiles(std::unordered_map<int64_t, Chunk>& world, int x, int y, const std::string& str, const std::vector<Color>& colors) {
	if (str.empty() || colors.empty()) return;

	bool singleColor = (colors.size() == 1);
	int cy = (y >= 0) ? (y / chunkH) : ((y - chunkH + 1) / chunkH);
	int ly = y - (cy * chunkH);

	if (ly < 0 || ly >= chunkH) return;
	int rowOffset = ly * chunkW;

	int currentX = x;
	size_t i = 0;

	while (i < str.size()) {
		int cx = (currentX >= 0) ? (currentX / chunkW) : ((currentX - chunkW + 1) / chunkW);
		int lx = currentX - (cx * chunkW);

		int64_t key = getKey(cx, cy);
		Chunk& chunk = world[key];

		int spaceInChunk = chunkW - lx;
		int charsToProcess = ((int)(str.size() - i) < spaceInChunk) ? (int)(str.size() - i) : spaceInChunk;

		for (int j = 0; j < charsToProcess; j++) {
			int idx = rowOffset + lx + j;
			chunk.codepoints[idx] = str[i + j];
			chunk.colors[idx] = singleColor ? colors[0] : colors[i + j];
		}

		i += charsToProcess;
		currentX += charsToProcess;
	}
}

void renderFile(std::unordered_map<int64_t, Chunk>& world, const std::vector<std::string>& content, 
		int x, int y) {
	static const std::vector<Color> RED_VEC = { RED };
	for (size_t i = 0; i < content.size(); i++) {
		changeTiles(world, x, y + i, content[i], RED_VEC);
	}
}
