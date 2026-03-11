#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <unordered_map>

constexpr int chunkW = 32;
constexpr int chunkH = 32;
constexpr int gapSize=3; //increase this to make the gaps smaller


namespace Colors {
	static constexpr SDL_Color WHITE = {240, 233, 201, 255};
	static constexpr SDL_Color BLACK = {53, 43, 64, 255};
	static constexpr SDL_Color GRAY = {125, 110, 110, 255};
	static constexpr SDL_Color DARKGREEN = {80, 141, 118, 255};
	static constexpr SDL_Color LIME = {118, 195, 121, 255};
	static constexpr SDL_Color YELLOW = {218, 203, 128, 255};
	static constexpr SDL_Color DARKBLUE = {83, 92, 137, 255};
	static constexpr SDL_Color PURPLE = {190, 121, 121, 255};
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
