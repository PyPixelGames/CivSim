#include "pathFinding.hpp"
#include "types.hpp"

// Use squared Euclidean — avoids sqrt entirely.
// Valid as a heuristic since relative ordering is preserved.
// If you need true distances in gScore, switch to this only in the heuristic.
static inline float heuristic(Pos a, Pos b) {
    float dx = (float)(a.x - b.x);
    float dy = (float)(a.y - b.y);
    // Octile distance — more accurate than Euclidean for 8-directional grids
    float adx = dx < 0 ? -dx : dx;
    float ady = dy < 0 ? -dy : dy;
    return (adx > ady) ? 0.414f * ady + adx : 0.414f * adx + ady;
}

static inline int64_t key(Pos p) {
    return (int64_t)p.x << 32 | (uint32_t)p.y;
}

std::vector<Pos> astar(Pos start, Pos goal,
                       std::unordered_map<int64_t, Chunk>& world,
                       float skipChecks, int maxNodes)
{
    if (start.x == goal.x && start.y == goal.y) return {};

    // Reserve upfront to avoid rehashing during search
    std::unordered_map<int64_t, float> gScore;
    std::unordered_map<int64_t, Pos>   cameFrom;
    gScore.reserve(512);
    cameFrom.reserve(512);

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;

    const int64_t startKey = key(start);
    const int64_t goalKey  = key(goal);

    gScore[startKey] = 0.0f;
    open.push({start, 0.0f, heuristic(start, goal)});

    constexpr int   dx[]   = {-1, 0, 1,-1, 1,-1, 0, 1};
    constexpr int   dy[]   = {-1,-1,-1, 0, 0, 1, 1, 1};
    constexpr float cost[] = {1.414f,1,1.414f,1,1,1.414f,1,1.414f};

    const bool doChecks = (skipChecks == 0.0f);
    int nodesExplored = 0;

    while (!open.empty()) {
        if (++nodesExplored > maxNodes) return {};

        Node cur = open.top(); open.pop();

        const int64_t curKey = key(cur.pos);

        // *** Key fix: skip stale entries left in the heap ***
        // Without this, outdated nodes get re-expanded, wasting work.
        auto gsIt = gScore.find(curKey);
        if (gsIt != gScore.end() && cur.g > gsIt->second) continue;

        if (curKey == goalKey) {
            std::vector<Pos> path;
            // Pre-size roughly to avoid repeated allocation
            path.reserve(64);
            Pos p = goal;
            const int64_t sk = startKey;
            while (key(p) != sk) {
                path.push_back(p);
                p = cameFrom[key(p)];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        const float curG = gsIt->second;

        for (int i = 0; i < 8; i++) {
            Pos nb{ cur.pos.x + dx[i], cur.pos.y + dy[i] };

            if (doChecks) {
                Cell c = checkCell(world, nb);
                if (c.bg.column == Tiles::Water ||
                    c.fg.column == Tiles::Tree) continue;
            }

            const int64_t nbKey  = key(nb);
            const float   tentG  = curG + cost[i];

            // Try to insert; if key exists, compare and update only if better.
            // Single lookup instead of count() + operator[].
            auto [it, inserted] = gScore.emplace(nbKey, tentG);
            if (!inserted) {
                if (tentG >= it->second) continue; // not better, skip
                it->second = tentG;                // update in place
            }

            cameFrom[nbKey] = cur.pos;
            open.push({nb, tentG, tentG + heuristic(nb, goal)});
        }
    }
    return {};
}
