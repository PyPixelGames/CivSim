#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <unordered_map>

constexpr int chunkW = 32;
constexpr int chunkH = 32;
constexpr int gapSize=0; //increase this to make the gaps smaller
constexpr int bakeSize=64;
constexpr float stride = bakeSize-gapSize;

constexpr SDL_Color bg = {25, 25, 25, 255};

namespace Colors {
	static constexpr SDL_Color WHITE = {255, 241, 232, 255};
	static constexpr SDL_Color BLACK = {0, 0, 0, 255};
	static constexpr SDL_Color LIGHTGRAY = {194, 195, 199, 255};
	static constexpr SDL_Color DARKGRAY = {95, 87, 79, 255};
	static constexpr SDL_Color DARKGREEN = {0, 135, 81, 255};
	static constexpr SDL_Color LIME = {0, 228, 54, 255};
	static constexpr SDL_Color YELLOW = {255, 236, 39, 255};
	static constexpr SDL_Color DARKBLUE = {29, 43, 83, 255};
	static constexpr SDL_Color PURPLE = {126, 37, 83, 255};
}

struct GameFont {
	TTF_Font* ttf      = nullptr;
	int       baseSize = 64;
	std::unordered_map<int, SDL_Texture*> glyphCache;
};

struct EditCell {
	int       x  = 0;
	int       y  = 0;
	char      ch = '?';
	SDL_Color c  = Colors::PURPLE;
};

struct Chunk {
	int codepoints[chunkW * chunkH];
	int ogCodepoints[chunkW * chunkH];
	SDL_Color colors [chunkW * chunkH];
	SDL_Color ogColors [chunkW * chunkH];
	SDL_Texture* tex = nullptr;
	std::vector<EditCell> cells;
};

struct ChunkCoord { int cx, cy, lx, ly; };
