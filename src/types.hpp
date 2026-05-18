#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <random>
#include <memory>

extern std::mt19937 rng;

constexpr int amount = 1;
constexpr int levelSizeX = 640*amount;
constexpr int levelSizeY = 640*amount;

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
	short int row=-1;
	short int column=-1;
	bool state=false;
};

struct Cell {
	AtlasPos fg;
	AtlasPos bg;
	AtlasPos entity;

	bool mask(const Cell& other) const{
		bool bgR=(other.bg.row==-1 || bg.row==other.bg.row);
		bool bgC=(other.bg.column==-1 || bg.column==other.bg.column);
		bool bgS=(bg.state==other.bg.state);

		bool fgR=(other.fg.row==-1 || fg.row==other.fg.row);
		bool fgC=(other.fg.column==-1 || fg.column==other.fg.column);
		bool fgS=(fg.state==other.fg.state);


		return (bgR&&bgC&&bgS && fgR&&fgC&&fgS);
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

	std::unordered_map<int, int> presentBGR;
	std::unordered_map<int, int> presentBGC;
	std::unordered_map<int, int> presentFGR;
	std::unordered_map<int, int> presentFGC;

	SDL_Texture* tex = nullptr;
	std::vector<EditCell> cells;

	void add(Cell& cell, int idx, bool og=false){
		c[idx] = cell;
		if (og==true){ ogC[idx]=cell;}
		if (cell.bg.row!=-1) presentBGR[cell.bg.row]++;
		if (cell.bg.column!=-1) presentBGC[cell.bg.column]++;
		if (cell.fg.row!=-1) presentFGR[cell.fg.row]++;
		if (cell.fg.column!=-1) presentFGC[cell.fg.column]++;
	}

	void deletePresense(Cell& cell){
		if (presentBGR[cell.bg.row]--==0) presentBGR.erase(cell.bg.row);
		if (presentBGC[cell.bg.column]--==0) presentBGC.erase(cell.bg.column);
		if (presentFGR[cell.fg.row]--==0) presentFGR.erase(cell.fg.row);
		if (presentFGC[cell.fg.column]--==0) presentFGC.erase(cell.fg.column);
	}

	bool find(Cell& mask){
		bool bgr=(mask.bg.row==-1 || presentBGR.count(mask.bg.row));
		bool bgc=(mask.bg.column==-1 || presentBGC.count(mask.bg.column));
		bool fgr=(mask.fg.row==-1 || presentFGR.count(mask.fg.row));
		bool fgc=(mask.fg.column==-1 || presentFGC.count(mask.fg.column));

		return (bgr && bgc && fgr && fgc);
	}
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
