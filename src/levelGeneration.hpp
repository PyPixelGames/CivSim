#pragma once

#include <iostream>
#include "types.hpp"
#include <unordered_map>
#include <cstdint>

void generateLevel(std::unordered_map<int64_t, Chunk>& world, int x, int y, Font font, int s);
