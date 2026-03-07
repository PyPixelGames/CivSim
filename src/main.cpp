#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include "types.hpp"
#include "helper.hpp"
#include "levelGeneration.hpp"
#include "mover.hpp"

using namespace Colors;

int main() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
		return 1;
	}
	if (!TTF_Init()) {
		std::cerr << "TTF_Init failed: " << SDL_GetError() << "\n";
		SDL_Quit();
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("SDL3 CivSim", 0, 0, SDL_WINDOW_FULLSCREEN);
	if (!window) {
		std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
		TTF_Quit(); SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer) {
		std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
		SDL_DestroyWindow(window); TTF_Quit(); SDL_Quit();
		return 1;
	}
	SDL_SetRenderVSync(renderer, 1);

	int width = 0, height = 0;
	SDL_GetRenderOutputSize(renderer, &width, &height);

	const int bakeSize = 64;
	GameFont font;
	font.baseSize = bakeSize;
	font.ttf = TTF_OpenFont("../src/fonts/AsciiFont.ttf", bakeSize);
	if (!font.ttf) {
		std::cerr << "TTF_OpenFont failed: " << SDL_GetError() << "\n";
		SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
		TTF_Quit(); SDL_Quit();
		return 1;
	}

	short int cellSize = 32;
	int worldX = 0;
	int worldY = 0;
	int sizeChange = 1;
	int movingSpeed = 5;

	std::unordered_map<int64_t, Chunk> world;

	auto dims = resize(cellSize, font);
	int cellW = dims.first;
	int cellH = dims.second;

	populateChunks(world, 30, 30, font, bakeSize);
	std::cout << "population complete\n";

	generateLevel(world, 0, 0, renderer, font, bakeSize);
	std::cout << "generation complete\n";

	EditCell ec;
	ec.x = 5; ec.y = 5;
	world[0].cells.push_back(ec);

	Uint64 fpsTimer = SDL_GetTicks();
	int frameCount = 0;
	float fps = 0.0f;

	bool running = true;
	SDL_Event event;

	Mover testMover(0, 0);

	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
			if (event.type == SDL_EVENT_KEY_DOWN) {
				if (event.key.key == SDLK_ESCAPE) running = false;
			}
			if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
				SDL_GetRenderOutputSize(renderer, &width, &height);
			}
		}

		const bool* keys = SDL_GetKeyboardState(nullptr);

		float mx, my;
		SDL_MouseButtonFlags buttons = SDL_GetMouseState(&mx, &my);

		if (buttons & SDL_BUTTON_LMASK) {
			cellSize += sizeChange;
			auto d = resize(cellSize, font);
			cellW = d.first; cellH = d.second;
		}
		if (buttons & SDL_BUTTON_RMASK) {
			if (cellSize - sizeChange > 8) {
				cellSize -= sizeChange;
				auto d = resize(cellSize, font);
				cellW = d.first; cellH = d.second;
			}
		}

		if (keys[SDL_SCANCODE_D]) worldX += movingSpeed;
		else if (keys[SDL_SCANCODE_A] && worldX >= movingSpeed) worldX -= movingSpeed;
		if (keys[SDL_SCANCODE_S]) worldY += movingSpeed;
		else if (keys[SDL_SCANCODE_W] && worldY >= movingSpeed) worldY -= movingSpeed;

		SDL_SetRenderTarget(renderer, nullptr);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);	

		testMover.update(world);

		int minChunkX = worldX / (chunkW * cellSize);
		int maxChunkX = minChunkX + (width  / (chunkW * cellSize)) + 2;
		int minChunkY = worldY / (chunkH * cellSize);
		int maxChunkY = minChunkY + (height / (chunkH * cellSize)) + 2;

		for (int cy = minChunkY; cy <= maxChunkY; cy++) {
			for (int cx = minChunkX; cx <= maxChunkX; cx++) {
				int64_t key = getKey(cx, cy);
				if (world.find(key) == world.end()) continue;

				Chunk& chunk = world[key];

				if (!chunk.cells.empty()){
					editTex(renderer, chunk, font, bakeSize);
					chunk.cells.clear();
				}

				float destX = (float)(cx * chunkW * cellSize - worldX);
				float destY = (float)(cy * chunkH * cellSize - worldY);
				float destW = (float)(chunkW * cellSize);
				float destH = (float)(chunkH * cellSize);

				SDL_FRect dst = {destX, destY, destW, destH};
				SDL_RenderTexture(renderer, chunk.tex, nullptr, &dst);
			}
		}

		frameCount++;
		Uint64 now = SDL_GetTicks();
		if (now - fpsTimer >= 500) {
			fps = (float)frameCount * 1000.0f / (float)(now - fpsTimer);
			frameCount = 0;
			fpsTimer = now;
		}
		drawFPS(renderer, font, fps, 1, 1);

		SDL_RenderPresent(renderer);
	}

	for (auto& [key, chunk] : world)
		if (chunk.tex) SDL_DestroyTexture(chunk.tex);

	for (auto& [cp, tex] : font.glyphCache)
		SDL_DestroyTexture(tex);

	TTF_CloseFont(font.ttf);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	std::cout << "unloaded\n";
	return 0;
}
