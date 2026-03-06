#pragma once
#include "types.hpp"
#include <unordered_map>
#include <cstdint>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>

int64_t getKey(int cx, int cy);

Chunk makeChunk(Font font, int s);

std::pair<int, int> resize(int cellSize, Font font);

void populateChunks(std::unordered_map<int64_t,Chunk>& world,int chunksX,int chunksY,Font font,int s);

std::vector<std::string> split(const std::string& s, char delim);

RenderTexture2D chunkTex(Chunk& chunk, Font font, int bakeSize);
