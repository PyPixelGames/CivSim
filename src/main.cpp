#include "civ.hpp"
#include "creature.hpp"
#include "helper.hpp"
#include "levelGeneration.hpp"
#include "logs.hpp"
#include "types.hpp"
#include "ui.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <vector>

using namespace TextColor;
unsigned int seed = std::random_device{}();
//unsigned int seed = 3781873310;
std::mt19937 rng(seed);

int main() {
	TeeBuf tee("src/logs.txt");
	std::cout.rdbuf(&tee);
	tee.clear();
	tee.setMode(LogMode::FileOnly);

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
		return 1;
	}
	if (!TTF_Init()) {
		std::cerr << "TTF_Init failed: " << SDL_GetError() << "\n";
		SDL_Quit();
		return 1;
	}

	SDL_Window *window =
		SDL_CreateWindow("SDL3 CivSim", 0, 0, SDL_WINDOW_FULLSCREEN);
	if (!window) {
		std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer) {
		std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	bool vsync = true;
	SDL_SetRenderVSync(renderer, vsync);

	int width = 0, height = 0;
	SDL_GetRenderOutputSize(renderer, &width, &height);

	float cellSize = 64;
	int worldX = 0;
	int worldY = 0;
	int sizeChange = 50;
	int movingSpeed = 500;

	bool updateFreze = true;
	bool nextUpdateStep = false;

	Uint64 lastTime = SDL_GetTicks();
	float deltaTime;
	float updateAccumulator = 0.0f;
	float updateInterval = 0.1f;
	float speedChange = 0.01f;
	const int maxUpdateChunks = 15;
	int chunksUpdated = 0;

	Uint64 fpsTimer = SDL_GetTicks();
	int frameCount = 0;
	float fps = 0.0f;

	bool running = true;
	SDL_Event event;

	std::unordered_map<int64_t, Chunk> world;

	float Mx, My;

	GameState state=GameState::GAME;
	uint32_t keyPressed;
	std::string inputText="";

	GameFont font;
	font.baseSize = bakeSize;
	font.ttf = TTF_OpenFont("src/fonts/AsciiFont.ttf", bakeSize);
	if (!font.ttf) {
		std::cerr << "TTF_OpenFont failed: " << SDL_GetError() << "\n";
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	TTF_Font *UIFont=TTF_OpenFont("src/fonts/inter.ttf", 28);
	if (!UIFont){
		std::cerr << "TTF_OpenFont failed: " << SDL_GetError() << "\n";
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	SDL_Texture *atlas = IMG_LoadTexture(renderer, "src/images/atlas.png");

	if (!atlas) {
		std::cerr << "Atlas load failed: " << SDL_GetError() << "\n";
	}

	std::cout << "Init fine\n" << std::endl;

	std::cout << "----- WORLD GENERATION -----\npopulation of chunks with blanks "
		"complete\n"
		<< std::endl;
	populateChunks(world, 100, 100);

	std::cout << "\n" << seed << std::endl;

	generateLevel(world, renderer, bakeSize, atlas);
	std::cout << "World generation complete\n-----\n" << std::endl;

	Civ testCiv;
	for (int i = 0; i < 1; i++) {
		Human *c = new Human({0, i}, world, testCiv.id, &testCiv);
		testCiv.pending.push_back(c);
		testCiv.id++;
	}

	std::vector<FloatingUI*> allUI;

	FloatingUI testUI;
	testUI.r.w=500;
	testUI.r.h=600;
	testUI.focused=true;

	auto testText=std::make_unique<UIPiece>();
	testText->name="This text looks fine";
	testUI.pieces.push_back(std::move(testText));
	allUI.push_back(&testUI);

	std::cout << "\n\n##########MAIN LOOP##########" << std::endl;
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
		if (deltaTime > 0.05f)
			deltaTime = 0.05f;

		SDL_GetMouseState(&Mx, &My);
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
			if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
				SDL_GetRenderOutputSize(renderer, &width, &height);
			}
			if (event.type == SDL_EVENT_KEY_DOWN) {
				if (event.key.key == SDLK_ESCAPE)
					running = false;
				keyPressed = event.key.key;
			}
			if (event.type == SDL_EVENT_TEXT_INPUT) {
				inputText=event.text.text;
			}
			if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					SDL_FPoint p = {Mx, My};
					bool clickedOnUI=false;
					for (auto ui: allUI){
						if (SDL_PointInRectFloat(&p, &ui->r)){
							ui->focused=true;
							clickedOnUI=true;
							break;
						}
					}
					//Return to game if no UI was pressed
					if (!clickedOnUI){
						state=GameState::GAME;
						for (auto ui: allUI){
							ui->focused=false;
						}
						if (SDL_TextInputActive(window)) {
							SDL_StopTextInput(window);
						}
						std::cout << std::endl;
					}
				}
			}
		}

		const bool *keys = SDL_GetKeyboardState(nullptr);

		float mx, my;
		SDL_MouseButtonFlags MouseButtons = SDL_GetMouseState(&mx, &my);

		auto zoomAround = [&](float newCellSize) {
			if (newCellSize < 8.0f)
				return;
			float scale = newCellSize / cellSize;
			worldX = (int)((worldX + mx) * scale - mx);
			worldY = (int)((worldY + my) * scale - my);
			if (worldX < 0)
				worldX = 0;
			if (worldY < 0)
				worldY = 0;
			cellSize = newCellSize;

			int newBakeSize;
			if (cellSize >= 32)
				newBakeSize = ogBakeSize;
			else if (cellSize >= 16)
				newBakeSize = ogBakeSize / 2;
			else
				newBakeSize = ogBakeSize / 16;

			if (newBakeSize != bakeSize) {
				bakeSize = newBakeSize;
				for (auto &[key, chunk] : world) {
					if (chunk.tex) {
						SDL_DestroyTexture(chunk.tex);
						chunk.tex = nullptr;
					}
				}
			}
		};

		if (state==GameState::GAME){
			if (MouseButtons & SDL_BUTTON_LMASK) {
				zoomAround(cellSize + sizeChange * deltaTime);
			}
			if (MouseButtons & SDL_BUTTON_RMASK) {
				zoomAround(cellSize - sizeChange * deltaTime);
			}

			if (keys[SDL_SCANCODE_D] && worldX < levelSizeX)
				worldX += (int)(movingSpeed * deltaTime);
			if (keys[SDL_SCANCODE_A] && worldX > 0)
				worldX -= (int)(movingSpeed * deltaTime);

			if (keys[SDL_SCANCODE_S] && worldY < levelSizeY)
				worldY += (int)(movingSpeed * deltaTime);
			if (keys[SDL_SCANCODE_W] && worldY > 0)
				worldY -= (int)(movingSpeed * deltaTime);

			if (keyPressed==SDLK_F) {
				updateFreze = !updateFreze;
				std::cout << "Changed updateFreze to: " << updateFreze << std::endl;
			}
			if (keyPressed==SDLK_T) {
				nextUpdateStep = true;
			}
			if (keyPressed==SDLK_E) {
				testCiv.evolve(world);
			}
			if (keyPressed==SDLK_I) {
				updateInterval += speedChange;
			}
			if (keyPressed==SDLK_O && updateInterval >= speedChange) {
				updateInterval -= speedChange;
			}
			if (keyPressed==SDLK_V && updateInterval >= speedChange) {
				vsync = !vsync;
				SDL_SetRenderVSync(renderer, vsync);
			}
		}else if (state==GameState::UI){
			if (inputText!=""){
				std::cout << inputText;
			}
		}

		SDL_SetRenderTarget(renderer, nullptr);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		int minChunkX = worldX / (chunkW * cellSize);
		int maxChunkX = minChunkX + (width / (chunkW * cellSize)) + 2;
		int minChunkY = worldY / (chunkH * cellSize);
		int maxChunkY = minChunkY + (height / (chunkH * cellSize)) + 2;

		updateVisibleChunks(world, renderer, atlas, minChunkX, maxChunkX, minChunkY,
				maxChunkY, bakeSize);

		for (int cy = minChunkY; cy <= maxChunkY; cy++) {
			for (int cx = minChunkX; cx <= maxChunkX; cx++) {
				int64_t key = getKey(cx, cy);
				if (world.find(key) == world.end())
					continue;

				Chunk &chunk = world[key];

				if (!chunk.cells.empty() && chunksUpdated < maxUpdateChunks) {
					editTex(renderer, chunk, bakeSize, atlas);
					chunk.cells.clear();
					chunksUpdated++;
				}

				float destW = (float)(chunkW * cellSize);
				float destH = (float)(chunkH * cellSize);

				float destX = (float)(cx * chunkW * cellSize - worldX);
				float destY = (float)(cy * chunkH * cellSize - worldY);

				SDL_FRect dst = {destX, destY, destW, destH};
				SDL_RenderTexture(renderer, chunk.tex, nullptr, &dst);
			}
		}

		drawFPS(renderer, font, fps, 1, 1);
		chunksUpdated = 0;

		// UPDATE LOOP

		// Update based on the tick counter
		updateAccumulator += deltaTime;
		bool updateState = updateAccumulator >= updateInterval && !updateFreze;
		if (updateState || nextUpdateStep) {
			updateAccumulator = 0;
			nextUpdateStep = false;

			// testCiv.evolve(world);
			testCiv.update(world);
		}

		// Always update
		bool uiState=false;
		for (auto ui: allUI){
			renderUI(renderer, *ui, UIFont);
			if(ui->focused){
				uiState=true;
				SDL_StartTextInput(window);
			}
		}

		//Update state
		if (uiState){
			state=GameState::UI;
		}else{
			state=GameState::GAME;
		}

		keyPressed=-1;
		inputText="";
		SDL_RenderPresent(renderer);
	}
	std::cout << "##########MAIN LOOP##########" << std::endl;

	testCiv.printStats();

	for (auto &[key, chunk] : world)
		if (chunk.tex)
			SDL_DestroyTexture(chunk.tex);

	for (auto &[cp, tex] : font.glyphCache)
		SDL_DestroyTexture(tex);

	TTF_CloseFont(font.ttf);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	std::cout << bold << "\nunloaded" << reset << std::endl;
	return 0;
}
