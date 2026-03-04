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
		c.codepoints[i] = '*';
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

void populateInitialChunks(std::unordered_map<int64_t, Chunk>& world,int worldX, int worldY,
		int screenW, int screenH, int cellW, int cellH){
	int visibleW = screenW / cellW;
	int visibleH = screenH / cellH;

	int minChunkX = worldX / chunkW;
	int maxChunkX = (worldX + visibleW) / chunkW;

	int minChunkY = worldY / chunkH;
	int maxChunkY = (worldY + visibleH) / chunkH;

	for (int cy = minChunkY; cy <= maxChunkY; cy++) {
		for (int cx = minChunkX; cx <= maxChunkX; cx++) {
			int64_t key = getKey(cx, cy);
			if (world.find(key) == world.end()) {
				world[key] = makeChunk();
			}
		}
	}
}

void changeTiles(std::unordered_map<int64_t, Chunk>& world,int x, int y,const std::string& str,
		const std::vector<Color>& colors){
	bool singleColor = (colors.size() == 1);

	int cx = x / chunkW;
	int cy = y / chunkH;
	int64_t key = getKey(cx, cy);

	Chunk& chunk = world[key];

	int lx = x - cx * chunkW;
	int ly = y - cy * chunkH;

	for (size_t i = 0; i < str.size(); i++) {
		int idx = ly * chunkW + (lx + i);
		chunk.codepoints[idx] = str[i];
		chunk.colors[idx] = singleColor ? colors[0] : colors[i];
	}
}

void renderFile(std::unordered_map<int64_t,Chunk>& world,std::vector<std::string>content,int x,int y){
	for (size_t i=0; i<content.size(); i++){
		changeTiles(world, x, y+i, content[i], std::vector<Color>{RED});
	}
}
