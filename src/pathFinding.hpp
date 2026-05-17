#pragma once
#include <queue>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "helper.hpp"

static inline float heuristic(Pos a, Pos b);

struct Node {
    Pos   pos;
    float g, f;
    bool operator>(const Node& o) const { return f > o.f; }
};

std::vector<Pos> astar(Pos start, Pos goal,
                       std::unordered_map<int64_t, Chunk>& world,
                       float skipChecks = 0.0f,
                       int   maxNodes   = 2000);
