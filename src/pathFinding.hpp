#pragma once

#include <queue>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

#include "helper.hpp"


// Heuristic — Euclidean works well for 8-directional movement
float heuristic(Pos a, Pos b);

struct Node {
	Pos pos;
	float g, f;
	bool operator>(const Node& o) const { return f > o.f; }
};

std::vector<Pos> astar(Pos start, Pos goal, std::unordered_map<int64_t, Chunk>& world,
                        float skipChecks=false,int maxNodes=2000);
