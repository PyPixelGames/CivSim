#pragma once
#include "types.hpp"
#include <unordered_map>
#include <cstdint>

void generateLevel(std::unordered_map<int64_t, Chunk>& world,
		int x, int y,
		SDL_Renderer* renderer,
		GameFont& font, int s);
