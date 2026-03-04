#pragma once
#include "types.hpp"
#include <unordered_map>
#include <cstdint>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>

int64_t getKey(int cx, int cy);

Chunk makeChunk();

std::pair<int, int> resize(int cellSize, Font font);

void populateChunks(std::unordered_map<int64_t, Chunk>& world,int chunksX, int chunksY);

void changeTiles(std::unordered_map<int64_t, Chunk>& world, int x, int y, const std::string& str, const std::vector<Color>& colors);

void renderFile(std::unordered_map<int64_t, Chunk>& world, const std::vector<std::string>& content, 
		int x, int y);

std::vector<std::string> split(const std::string& s, char delim);

std::vector<std::string> loadFile(std::string filename, char delim='g');
