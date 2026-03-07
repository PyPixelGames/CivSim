#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <unordered_map>

constexpr int chunkW = 32;
constexpr int chunkH = 32;

namespace Colors {
	static constexpr SDL_Color WHITE     = {255, 255, 255, 255};
	static constexpr SDL_Color BLACK     = {  0,   0,   0, 255};
	static constexpr SDL_Color GRAY      = {130, 130, 130, 255};
	static constexpr SDL_Color DARKGREEN = {  0, 117,  44, 255};
	static constexpr SDL_Color LIME      = {  0, 158,  47, 255};
	static constexpr SDL_Color BEIGE     = {211, 176, 131, 255};
	static constexpr SDL_Color DARKBLUE  = {  0,  82, 172, 255};
	static constexpr SDL_Color PURPLE    = {200, 122, 255, 255};
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
	int          codepoints[chunkW * chunkH];
	SDL_Color    colors    [chunkW * chunkH];
	SDL_Texture* tex = nullptr;
	std::vector<EditCell> cells;
};
