#include "raylib.h"
#include <iostream>
#include <unordered_map>
#include <cstdint>
#include <vector>

constexpr int chunkW = 32;
constexpr int chunkH = 32;

struct Chunk {
	int tiles[chunkW * chunkH];
	Color colors[chunkW * chunkH];
};

int64_t getKey(int cx, int cy) {
	return (int64_t(cx) << 32) | int64_t(uint32_t(cy));
}

Chunk makeChunk() {
	Chunk c;
	for (int i = 0; i < chunkW * chunkH; i++) {
		c.tiles[i] = '`';
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
		int screenW, int screenH,
		int cellW, int cellH){
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

void changeTile(std::unordered_map<int64_t, Chunk>& world, int x, int y, int codepoint, Color color){
	int cx = x / chunkW;
	int cy = y / chunkH;
	int lx = x - cx * chunkW;
	int ly = y - cy * chunkH;
	int idx = ly * chunkW + lx;
	int64_t key = getKey(cx, cy);
	world[key].tiles[idx] = codepoint;
	world[key].colors[idx] = color;
}


int main() {
	SetConfigFlags(FLAG_FULLSCREEN_MODE);
	InitWindow(0, 0, "raylib window");

	int width = GetScreenWidth();
	int height = GetScreenHeight();

	const int baseFontSize = 256;

	std::vector<int> codepoints;

	// ASCII
	for (int i = 32; i < 127; i++) codepoints.push_back(i);

	Font jet = LoadFontEx("../src/fonts/AsciiFont.ttf",baseFontSize,
			codepoints.data(),codepoints.size());
	SetTextureFilter(jet.texture, TEXTURE_FILTER_POINT);
	Shader sdfShader = LoadShader(0, "shaders/sdf.fs");


	short int cellSize = 32;
	int worldX = 0; int worldY = 0;

	std::unordered_map<int64_t, Chunk> world;

	auto [cellW, cellH] = resize(cellSize, jet);

	populateInitialChunks(world, worldX, worldY, width, height, cellW, cellH);
	
	changeTile(world, 4, 5, 'R', PURPLE);
	changeTile(world, 5, 5, 'E', RED);
	changeTile(world, 6, 5, 'D', GREEN);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		BeginShaderMode(sdfShader);

		int visibleW = width / cellW;
		int visibleH = height / cellH;

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

				Chunk &chunk = world[key];

				int chunkOffsetX = cx * chunkW;
				int chunkOffsetY = cy * chunkH;

				for (int y = 0; y < chunkH; y++) {
					for (int x = 0; x < chunkW; x++) {
						int wx = chunkOffsetX + x;
						int wy = chunkOffsetY + y;

						if (wx < worldX || wy < worldY ||
								wx >= worldX + visibleW ||
								wy >= worldY + visibleH){
							continue;
						}

						int sx = (wx - worldX) * cellW;
						int sy = (wy - worldY) * cellH;

						int idx = y * chunkW + x;

						DrawTextCodepoint(jet, chunk.tiles[idx], {(float)sx, (float)sy}, cellSize,
								chunk.colors[idx]);
					}
				}
			}
		}

		EndShaderMode();
		EndDrawing();
	}

	UnloadShader(sdfShader);
	UnloadFont(jet);
	CloseWindow();

	return 0;
}
