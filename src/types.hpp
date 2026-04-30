#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

constexpr int chunkW = 32;
constexpr int chunkH = 32;
constexpr int gapSize=0; //increase this to make the gaps smaller
constexpr int ogBakeSize=64;
inline int bakeSize=64;
inline float stride = bakeSize-gapSize;

constexpr SDL_Color bg = {25, 25, 25, 255};

namespace TextColor {
    const std::string reset   = "\033[0m";
    const std::string bold    = "\033[1m";
    const std::string red     = "\033[31m";
    const std::string darkRed = "\033[38;5;88m";
    const std::string green   = "\033[32m";
    const std::string yellow  = "\033[33m";
    const std::string blue    = "\033[34m";
    const std::string magenta = "\033[35m";
    const std::string cyan    = "\033[36m";
}


typedef struct {
	int name;
	SDL_FRect uv;
} NamedEntry;

static constexpr NamedEntry tiles[] = {
	{0, {0, 0, 64, 64}},
	{1, {64, 0, 64, 64}},
	{2, {128, 0, 64, 64}},
	{3, {64*3, 0, 64, 64}},
	{4, {64*4, 0, 64, 64}},
	{5, {64*5, 0, 64, 64}},
};

enum Terrain{
	Water = 0,
	Grass = 1,
	Tree = 2,
	Sand = 3,
	Mountain = 4,
	MountainTop = 5,

	FIELDS=0,
	SNOWY=1,
};

struct AtlasPos{
	short int row=0;
	short int column=0;
	bool state=false;
};

struct Cell {
	AtlasPos fg;
	AtlasPos bg;
};

struct GameFont {
	TTF_Font* ttf = nullptr;
	int baseSize = 64;
	std::unordered_map<int, SDL_Texture*> glyphCache;
	SDL_Texture* fpsTex = nullptr;
	std::string lastFpsText;
};

struct EditCell {
	int x = 0;
	int y = 0;
	Cell c;
};

struct Chunk {
	Cell c [chunkW*chunkH];
	Cell ogC [chunkW*chunkH];

	SDL_Texture* tex = nullptr;
	std::vector<EditCell> cells;
};

struct ChunkCoord { int cx, cy, lx, ly, idx; };

struct Pos {
	int x, y;

	bool operator==(const Pos& other) const {
		return x == other.x && y == other.y;
	}

	int distance(Pos& other) {
		int output = ((x-other.x)+(y-other.y))/2;

		if (output>0){
			return output;
		}else{
			return output*-1;
		}
	}
};
