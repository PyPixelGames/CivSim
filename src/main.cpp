#include <iostream>
#include <raylib.h>
#include <vector>
#include <string>

#include "types.hpp"
#include "helper.hpp"
#include "levelGeneration.hpp"

int main() {
	SetConfigFlags(FLAG_FULLSCREEN_MODE);
	InitWindow(0, 0, "raylib window");

	int width = GetScreenWidth();
	int height = GetScreenHeight();

	const int baseFontSize = 256;

	std::vector<int> codepoints;

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

	int generateSize = 32;
	populateChunks(world, 30, 30, font, 32);

	std::cout<<"population complete"<<std::endl;

	generateLevel(world, 0, 0, font, 32);
	std::cout << "generation complete" << std::endl;

	int sizeChange=1;
	int movingSpeed=5;

	while (!WindowShouldClose()) {
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			cellSize += sizeChange;
			std::pair<int,int> dims = resize(cellSize, font);
			cellW = dims.first;
			cellH = dims.second;
		}
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
			if (cellSize - sizeChange > 8) {
				cellSize -= sizeChange;
				std::pair<int,int> dims = resize(cellSize, font);
				cellW = dims.first;
				cellH = dims.second;
			}
		}
		if (IsKeyDown(KEY_D)){
			worldX+=movingSpeed;	
		}else if (IsKeyDown(KEY_A) && worldX>=movingSpeed){
			worldX-=movingSpeed;	
		}if (IsKeyDown(KEY_S)){
			worldY+=movingSpeed;	
		}else if (IsKeyDown(KEY_W) && worldY>=movingSpeed){
			worldY-=movingSpeed;		
		}


		BeginDrawing();
		ClearBackground(BLACK);

		int pixelOffsetX = worldX % (chunkW * cellSize);
		int pixelOffsetY = worldY % (chunkH * cellSize);

		int minChunkX = worldX / (chunkW * cellSize);
		int maxChunkX = minChunkX + (width / (chunkW * cellSize)) + 2;
		int minChunkY = worldY / (chunkH * cellSize);
		int maxChunkY = minChunkY + (height / (chunkH * cellSize)) + 2;

		for (int cy = minChunkY; cy <= maxChunkY; cy++) {
			for (int cx = minChunkX; cx <= maxChunkX; cx++) {
				int64_t key = getKey(cx, cy);
				if (world.find(key) == world.end()) continue;

				Chunk& chunk = world[key];

				int destX = cx * chunkW * cellSize - worldX;
				int destY = cy * chunkH * cellSize - worldY;
				int destW = chunkW * cellSize;
				int destH = chunkH * cellSize;

				Rectangle src = {
					0.0f, 0.0f,
					(float)chunk.tex.texture.width,
					-(float)chunk.tex.texture.height
				};
				Rectangle dst = {
					(float)destX, (float)destY,
					(float)destW, (float)destH
				};

				DrawTexturePro(chunk.tex.texture, src, dst, {0, 0}, 0.0f, WHITE);
			}
		}
		DrawFPS(1, 1);

		EndDrawing();
	}

	UnloadShader(sdfShader);
	UnloadFont(font);
	for (auto& [key, chunk] : world){
		UnloadRenderTexture(chunk.tex);
	}
	CloseWindow();  // last

	std::cout << "unloaded" << std::endl;

	return 0;
}
