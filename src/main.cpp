#include <iostream>
#include <vector>
#include <algorithm>

#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "civ.hpp"
#include "creature.hpp"
#include "helper.hpp"
#include "levelGeneration.hpp"
#include "logs.hpp"
#include "types.hpp"
#include "ui.hpp"

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

	SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
	const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(displayID);

	int screenW = mode ? mode->w : 1920;
	int screenH = mode ? mode->h : 1080;

	SDL_Window *window = SDL_CreateWindow("SDL3 CivSim", screenW, screenH, SDL_WINDOW_FULLSCREEN);
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
	SDL_SetRenderLogicalPresentation(renderer, 1920, 1080, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	int width = 0, height = 0;
	SDL_GetWindowSize(window, &width, &height);

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

	bool stateSwitch=false;

	std::unordered_map<int64_t, Chunk> world;

	Mouse mouse;
	Keyboard keyboard;

	GameState state=GameState::GAME;

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

		SDL_MouseButtonFlags MouseButtons=SDL_GetMouseState(&mouse.pos.x, &mouse.pos.y);
		mouse.holdingLeft=MouseButtons & SDL_BUTTON_LMASK;
		mouse.holdingRight=MouseButtons & SDL_BUTTON_RMASK;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
			if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
				SDL_GetRenderOutputSize(renderer, &width, &height);
			}
			if (event.type == SDL_EVENT_KEY_DOWN) {
				keyboard.keyPressed = event.key.key;

				if (event.key.key == SDLK_ESCAPE){
					running=false;
				}
			}
			if (event.type == SDL_EVENT_TEXT_INPUT) {
				keyboard.input=event.text.text;
			}
			if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				if (event.button.button == SDL_BUTTON_RIGHT){
					if (state==GameState::GAME){
						Pos selectedCell = {
							static_cast<int>((mouse.pos.x + worldX) / cellSize),
							static_cast<int>((mouse.pos.y + worldY) / cellSize)
						};

						FloatingUI* cellUI = new FloatingUI();
						cellUI->focused = true;
						cellUI->singleUse = true;

						cellUI->originPos = {
							static_cast<int>((selectedCell.x * cellSize) - worldX + (cellSize / 2.0f)),
							static_cast<int>((selectedCell.y * cellSize) - worldY + (cellSize / 2.0f))
						};

						cellUI->r.w = 300;
						cellUI->r.h = 300;

						int offset = chunkW;

						if (mouse.pos.x <= static_cast<int>(screenW / 2)) {
							cellUI->r.x = std::floor(mouse.pos.x + offset);
						} else {
							cellUI->r.x = std::floor(mouse.pos.x - offset - cellUI->r.w);
						}

						if (mouse.pos.y <= static_cast<int>(screenH / 2)) {
							cellUI->r.y = std::floor(mouse.pos.y + offset);
						} else {
							cellUI->r.y = std::floor(mouse.pos.y - offset - cellUI->r.h);
						}

						allUI.push_back(cellUI);


						Cell c = checkCell(world, selectedCell);

						auto cellInfo = std::make_unique<UIPiece>();
						cellInfo->relativePos = {55, 25};
						std::string s = std::to_string(selectedCell.x) + "  " + std::to_string(selectedCell.y);
						cellInfo->name = s;
						cellUI->pieces.push_back(std::move(cellInfo));

						auto cellAtlasBGR = std::make_unique<UIPiece>();
						cellAtlasBGR->type=UIType::INPUT;
						cellAtlasBGR->relativePos={10, 50};
						cellAtlasBGR->width=50;
						cellAtlasBGR->name = std::to_string(c.bg.row);

						auto cellAtlasBGC = std::make_unique<UIPiece>(*cellAtlasBGR);
						cellAtlasBGC->relativePos={110, 50};
						cellAtlasBGC->name = std::to_string(c.bg.column);

						auto cellAtlasFGB = std::make_unique<UIPiece>(*cellAtlasBGR);
						cellAtlasFGB->relativePos={10, 150};
						cellAtlasFGB->name = std::to_string(c.fg.row);

						auto cellAtlasFGC = std::make_unique<UIPiece>(*cellAtlasBGR);
						cellAtlasFGC->relativePos={110, 150};
						cellAtlasFGC->name = std::to_string(c.fg.column);

						cellUI->pieces.push_back(std::move(cellAtlasBGR));
						cellUI->pieces.push_back(std::move(cellAtlasBGC));
						cellUI->pieces.push_back(std::move(cellAtlasFGB));
						cellUI->pieces.push_back(std::move(cellAtlasFGC));


						auto submit = std::make_unique<UIPiece>();
						submit->type = UIType::BUTTON;
						submit->relativePos = {55, 230};
						submit->function = [&world, cellUI, selectedCell](){
							int buttonStart=1;
							Cell change;
							change.bg.row=std::stoi(cellUI->pieces[buttonStart]->name);
							change.bg.column=std::stoi(cellUI->pieces[buttonStart+1]->name);
							change.fg.row=std::stoi(cellUI->pieces[buttonStart+2]->name);
							change.fg.column=std::stoi(cellUI->pieces[buttonStart+3]->name);

							if (change.bg.row!=-1 && change.bg.column!=-1) change.bg.state=true;
							if (change.fg.row!=-1 && change.fg.column!=-1) change.fg.state=true;

							changeCell(world, selectedCell, change);
							std::cout << change.bg.row << " " << change.bg.column << std::endl;
						};

						cellUI->pieces.push_back(std::move(submit));

					}
				}
				if (event.button.button == SDL_BUTTON_LEFT) {
					mouse.left = {static_cast<int>(mouse.pos.x), static_cast<int>(mouse.pos.y)};

					int index=-1;
					for (int i = 0; i < allUI.size(); i++) {
						auto& ui = allUI[i];
						if (SDL_PointInRectFloat(&mouse.pos, &ui->r) && ui->open){
							ui->focusPieces(&mouse.pos);
							ui->focused=true;
							ui->dirty=true;
							index=i;
							break;
						}
					}

					//Return to game if no UI was pressed
					for (int i = 0; i < allUI.size(); i++) {
						if (i!=index){
							auto& ui = allUI[i];
							if (ui->focused){
								ui->unfocus();
								//ui->open=false;
							}
						}
					}
					if (index == -1){
						if (state==GameState::UI){
							state=GameState::GAME;
							stateSwitch=true;
							mouse.holdingLeft=true;
							if (SDL_TextInputActive(window)) {
								SDL_StopTextInput(window);
							}
						}
					}
				}

			}
		}

		keyboard.keysHeld = SDL_GetKeyboardState(nullptr);
		if (!mouse.holdingLeft && !mouse.holdingRight && stateSwitch) {
			stateSwitch = false;
		}

		if (state==GameState::GAME){
			auto zoomAround = [&](float newCellSize) {
				if (newCellSize < 8.0f)
					return;
				float scale = newCellSize / cellSize;
				worldX = (int)((worldX + mouse.pos.x) * scale - mouse.pos.x);
				worldY = (int)((worldY + mouse.pos.y) * scale - mouse.pos.y);
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


			if (keyboard.keysHeld[SDL_SCANCODE_E] && !stateSwitch) {
				zoomAround(cellSize + sizeChange * deltaTime);
			}
			if (keyboard.keysHeld[SDL_SCANCODE_Q] && !stateSwitch) {
				zoomAround(cellSize - sizeChange * deltaTime);
			}

			if (keyboard.keysHeld[SDL_SCANCODE_D] && worldX < (int)(levelSizeX * cellSize))
				worldX += (int)(movingSpeed * deltaTime);
			if (keyboard.keysHeld[SDL_SCANCODE_A] && worldX > 0)
				worldX -= (int)(movingSpeed * deltaTime);

			if (keyboard.keysHeld[SDL_SCANCODE_S] && worldY < (int)(levelSizeY * cellSize))
				worldY += (int)(movingSpeed * deltaTime);
			if (keyboard.keysHeld[SDL_SCANCODE_W] && worldY > 0)
				worldY -= (int)(movingSpeed * deltaTime);

			if (keyboard.keyPressed==SDLK_F) {
				updateFreze = !updateFreze;
				std::cout << "Changed updateFreze to: " << updateFreze << std::endl;
			}
			if (keyboard.keyPressed==SDLK_T) {
				nextUpdateStep = true;
			}
			if (keyboard.keyPressed==SDLK_E) {
				testCiv.evolve(world);
			}
			if (keyboard.keyPressed==SDLK_I) {
				updateInterval += speedChange;
			}
			if (keyboard.keyPressed==SDLK_O && updateInterval >= speedChange) {
				updateInterval -= speedChange;
			}
			if (keyboard.keyPressed==SDLK_V && updateInterval >= speedChange) {
				vsync = !vsync;
				SDL_SetRenderVSync(renderer, vsync);
			}
		}else if (state==GameState::UI){
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

			if (state==GameState::GAME){
				// testCiv.evolve(world);
				testCiv.update(world);
			}
		}

		// Always update
		bool uiState=false;
		for (auto ui: allUI){
			if (ui->originPos!=Pos{-1, -1}){
				setColor(renderer, {255, 0, 0, 255});
				SDL_RenderLine(renderer,
						ui->r.x+ui->r.w/2,
						ui->r.y+ui->r.h/2,
						ui->originPos.x,
						ui->originPos.y);
			}

			updateUI(renderer, *ui, UIFont, mouse, keyboard);
			if(ui->focused){
				uiState=true;
				SDL_StartTextInput(window);
			}
		}

		allUI.erase(std::remove_if(allUI.begin(), allUI.end(), [](const FloatingUI* ui){
					return ui->singleUse && !ui->focused;
					}), allUI.end());

		//Update state
		if (uiState){
			state=GameState::UI;
		}else{
			state=GameState::GAME;
		}


		mouse.reset();
		keyboard.reset();
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
