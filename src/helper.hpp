#pragma once
#include "types.hpp"
#include <SDL3_ttf/SDL_ttf.h>
#include <unordered_map>
#include <cstdint>
#include <random>

int64_t getKey(int cx, int cy);

std::pair<int,int> fromKey(int64_t key);

Chunk makeChunk();
void populateChunks(std::unordered_map<int64_t, Chunk>& world,
		int chunksX, int chunksY);

SDL_Texture* chunkTex(SDL_Renderer* renderer, Chunk& chunk, int bakeSize, SDL_Texture* atlas);

void editTex(SDL_Renderer* renderer, Chunk& chunk,int bakeSize, SDL_Texture* atlas);

void drawFPS(SDL_Renderer* renderer, GameFont& font,
		float fps, int x, int y);

ChunkCoord toChunk(int x, int y);

void updateVisibleChunks(std::unordered_map<int64_t, Chunk>& world, SDL_Renderer* renderer,
		SDL_Texture* atlas,int minCX,int maxCX,int minCY,int maxCY,int bakeSize);


void changeCell(std::unordered_map<int64_t, Chunk>& world,Pos p,Cell c,bool restore=false);

Cell checkCell(std::unordered_map<int64_t, Chunk>& world, Pos p, bool og=false);

float clampFloat(float f, float l, float h);

SDL_Texture* renderText(SDL_Renderer* renderer, std::string string, TTF_Font* font, SDL_Color color);

void setColor(SDL_Renderer* renderer, SDL_Color c);
