#pragma once
#include "types.hpp"
#include <unordered_map>
#include <cstdint>

void generateLevel(std::unordered_map<int64_t, Chunk>& world,SDL_Renderer* renderer, int s);
