#include <iostream>
#include <unordered_map>
#include <cstdint>
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

	populateChunks(world, 340, 340);

	std::cout<<"population complete"<<std::endl;

	std::vector<std::string> file = loadFile("../src/levels/test.txt");
	renderFile(world, generateLevel(), 0, 0);

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

				//if (world.find(key) == world.end()) {
				//	world[key] = makeChunk();
				//}

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

						DrawTextCodepoint(font, chunk.codepoints[idx],{(float)sx,(float)sy},cellSize,
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
