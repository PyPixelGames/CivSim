#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>
#include <string>
#include "types.hpp"
#include "helper.hpp"
#include "levelGeneration.hpp"
#include "civ.hpp"

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

	float cellSize = 32;
	int worldX = 0;
	int worldY = 0;
	int sizeChange = 50;
	int movingSpeed = 300;

	std::unordered_map<int64_t, Chunk> world;

	GameFont font;
	font.baseSize = bakeSize;
	font.ttf = TTF_OpenFont("../src/fonts/AsciiFont.ttf", bakeSize);
	if (!font.ttf) {
		std::cerr << "TTF_OpenFont failed: " << SDL_GetError() << "\n";
		SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
		TTF_Quit(); SDL_Quit();
		return 1;
	}

	SDL_Texture *atlas = IMG_LoadTexture(renderer, "../src/images/atlas.png");	

	if (!atlas) {
		std::cerr << "Atlas load failed: " << SDL_GetError() << "\n";
	}

	populateChunks(world, 100, 100, bakeSize);
	std::cout << "population complete\n";

	generateLevel(world, renderer, bakeSize, atlas);
	std::cout << "generation complete\n";

	Uint64 fpsTimer = SDL_GetTicks();
	int frameCount = 0;
	float fps = 0.0f;

	bool running = true;
	SDL_Event event;

	Civ testCiv;

	for (size_t i=0; i<5; i++){
		Creature* c = new Creature({0, 0}, world);
		testCiv.creatures.push_back(c);
	}

	Uint64 lastTime = SDL_GetTicks();
	float deltaTime;
	float updateAccumulator = 0.0f;
	const float updateInterval = 0.1f;

	while (running) {
		frameCount++;
		Uint64 now = SDL_GetTicks();
		if (now - fpsTimer >= 500) {
			fps = (float)frameCount * 1000.0f / (float)(now - fpsTimer);
			frameCount = 0;
			fpsTimer = now;
		}
		deltaTime = (now - lastTime) / 1000.0f;
		lastTime = now;
		// clamp to avoid spiral of death on lag spikes
		if (deltaTime > 0.05f) deltaTime = 0.05f;

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

		auto zoomAround = [&](float newCellSize) {
			if (newCellSize < 8.0f) return;
			float scale = newCellSize / cellSize;
			worldX = (int)((worldX + mx) * scale - mx);
			worldY = (int)((worldY + my) * scale - my);

			if (worldX < 0){
				worldX = 0;
			}

			if (worldY < 0){
				worldY = 0;
			}

			cellSize = newCellSize;
		};

		if (buttons & SDL_BUTTON_LMASK) {
			zoomAround(cellSize + sizeChange * deltaTime);
		}
		if (buttons & SDL_BUTTON_RMASK) {
			zoomAround(cellSize - sizeChange * deltaTime);
		}

		if (keys[SDL_SCANCODE_D]) worldX += (int)(movingSpeed * deltaTime);
		if (keys[SDL_SCANCODE_A] && worldX > 0) worldX -= (int)(movingSpeed * deltaTime);

		if (keys[SDL_SCANCODE_S]) worldY += (int)(movingSpeed * deltaTime);
		if (keys[SDL_SCANCODE_W] && worldY > 0) worldY -= (int)(movingSpeed * deltaTime);

		SDL_SetRenderTarget(renderer, nullptr);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);	

		updateAccumulator += deltaTime;
		if (updateAccumulator >= updateInterval) {
			updateAccumulator -= updateInterval;

			testCiv.update(world);
		}

		int minChunkX = worldX / (chunkW * (cellSize-gapSize));
		int maxChunkX = minChunkX + (width  / (chunkW * (cellSize-gapSize))) + 2;
		int minChunkY = worldY / (chunkH * (cellSize-gapSize));
		int maxChunkY = minChunkY + (height / (chunkH * (cellSize-gapSize))) + 2;

		updateVisibleChunks(world, renderer, atlas,
				minChunkX, maxChunkX, minChunkY, maxChunkY, bakeSize);  

		for (int cy = minChunkY; cy <= maxChunkY; cy++) {
			for (int cx = minChunkX; cx <= maxChunkX; cx++) {
				int64_t key = getKey(cx, cy);
				if (world.find(key) == world.end()) continue;

				Chunk& chunk = world[key];

				if (!chunk.cells.empty()){
					editTex(renderer, chunk, bakeSize, atlas);
					chunk.cells.clear();
				}

				float destW = (float)(chunkW * (cellSize - gapSize));
				float destH = (float)(chunkH * (cellSize - gapSize));

				float destX = (float)(cx * chunkW * (cellSize - gapSize) - worldX);
				float destY = (float)(cy * chunkH * (cellSize - gapSize) - worldY);

				SDL_FRect dst = {destX, destY, destW, destH};
				SDL_RenderTexture(renderer, chunk.tex, nullptr, &dst);
			}
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
	testCiv.clear();

	std::cout << "unloaded\n";
	return 0;
}
