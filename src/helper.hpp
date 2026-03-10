#pragma once
#include "types.hpp"
#include <unordered_map>
#include <cstdint>
#include <string>
#include <vector>

int64_t getKey(int cx, int cy);

Chunk makeChunk(GameFont& font, int s);

void populateChunks(std::unordered_map<int64_t, Chunk>& world,
		int chunksX, int chunksY,
		GameFont& font, int s);

void drawGlyph(SDL_Renderer* renderer, GameFont& font,
		int codepoint, float x, float y, int cellSize,
		SDL_Color color);

SDL_Texture* chunkTex(SDL_Renderer* renderer, Chunk& chunk,
		GameFont& font, int bakeSize);

void editTex(SDL_Renderer* renderer, Chunk& chunk,
		GameFont& font, int bakeSize);

void drawFPS(SDL_Renderer* renderer, GameFont& font,
		float fps, int x, int y);
