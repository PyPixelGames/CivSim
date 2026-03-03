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
		c.tiles[i] = '*';
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


void changeTiles(std::unordered_map<int64_t, Chunk>& world, int x, int y, const std::string& str,
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
		chunk.tiles[idx] = (int)str[i];
		chunk.colors[idx] = singleColor ? colors[0] : colors[i];
	}
}


int main() {
	SetConfigFlags(FLAG_FULLSCREEN_MODE);
	InitWindow(0, 0, "raylib window");

	int width = GetScreenWidth();
	int height = GetScreenHeight();

	const int baseFontSize = 64;

	std::vector<int> codepoints;

	std::cout << "Working dir: " << GetWorkingDirectory() << "\n";


	for (int i = 32; i < 127; i++) codepoints.push_back(i);
	Font font = LoadFontEx("../src/fonts/AsciiFont.ttf",baseFontSize,
			codepoints.data(),codepoints.size());
	SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
	Shader sdfShader = LoadShader(0, "/home/luka/projects/CivSim/src/shaders/sdf.fs");


	short int cellSize = 32;
	int worldX = 0; int worldY = 0;

	std::unordered_map<int64_t, Chunk> world;

	int cellW, cellH;

	std::pair<int,int> dims = resize(cellSize, font);
	cellW = dims.first;
	cellH = dims.second;

	populateInitialChunks(world, worldX, worldY, width, height, cellW, cellH);

	changeTiles(world, 5, 5, "---", std::vector<Color>{RED});
	changeTiles(world, 5, 6, "|#|", std::vector<Color>{RED, GREEN, RED});
	changeTiles(world, 5, 7, "---", std::vector<Color>{RED});

	int sizeChange=4;
	int movingSpeed=1;

	while (!WindowShouldClose()) {
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			cellSize+=sizeChange;

			std::pair<int,int> dims = resize(cellSize, font);
			cellW = dims.first;
			cellH = dims.second;
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			if(cellSize-sizeChange>8){
				cellSize-=sizeChange;

				std::pair<int,int> dims = resize(cellSize, font);
				cellW = dims.first;
				cellH = dims.second;
			}
		}
		if (IsKeyPressed(KEY_D)){
			worldX+=movingSpeed;	
		}else if (IsKeyPressed(KEY_A) && worldX>=movingSpeed){
			worldX-=movingSpeed;		
		}if (IsKeyPressed(KEY_S)){
			worldY+=movingSpeed;	
		}else if (IsKeyPressed(KEY_W) && worldY>=movingSpeed){
			worldY-=movingSpeed;		
		}


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

						DrawTextCodepoint(font, chunk.tiles[idx], {(float)sx, (float)sy}, cellSize,
								chunk.colors[idx]);
					}
				}
			}
		}

		EndShaderMode();
		EndDrawing();
	}

	UnloadShader(sdfShader);
	UnloadFont(font);
	CloseWindow();

	return 0;
}
