#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <random>

extern std::mt19937 rng;

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


enum Tiles{
	Water = 0,
	Grass = 1,
	Tree = 2,
	Sand = 3,
	Mountain = 4,
	MountainTop = 5,

	bush = 6,

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

struct CellHash {
    size_t operator()(const Cell& c) const {
        size_t h = 0;
        h ^= std::hash<short>()(c.fg.row) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= std::hash<short>()(c.fg.column) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= std::hash<short>()(c.bg.row) + 0x9e3779b9 + (h<<6) + (h>>2);
        h ^= std::hash<short>()(c.bg.column) + 0x9e3779b9 + (h<<6) + (h>>2);
        return h;
    }
};

struct CellEqual {
    bool operator()(const Cell& a, const Cell& b) const {
        return a.fg.row == b.fg.row && a.fg.column == b.fg.column &&
               a.bg.row == b.bg.row && a.bg.column == b.bg.column;
    }
};

struct EditCell {
	int x = 0;
	int y = 0;
	Cell c;
};

struct GameFont {
	TTF_Font* ttf = nullptr;
	int baseSize = 64;
	std::unordered_map<int, SDL_Texture*> glyphCache;
	SDL_Texture* fpsTex = nullptr;
	std::string lastFpsText;
};

struct Chunk {
	Cell c [chunkW*chunkH];
	Cell ogC [chunkW*chunkH];

	std::unordered_map<Cell, int, CellHash, CellEqual> cellCount;

	SDL_Texture* tex = nullptr;
	std::vector<EditCell> cells;
};

struct ChunkCoord { int cx, cy, lx, ly, idx; };

struct Pos {
	int x=0, y=0;

	bool operator==(const Pos& other) const {
		return x == other.x && y == other.y;
	}

	int distance(const Pos& other) const {
		return std::abs(x - other.x) + std::abs(y - other.y);
	}
};
