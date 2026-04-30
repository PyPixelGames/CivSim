#include "pathFinding.hpp"
#include "types.hpp"

// Heuristic — Euclidean works well for 8-directional movement
float heuristic(Pos a, Pos b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

std::vector<Pos> astar(Pos start, Pos goal, std::unordered_map<int64_t, Chunk>& world,
                        float skipChecks,int maxNodes){
    if (start.x == goal.x && start.y == goal.y) return {};

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    std::unordered_map<int64_t, float> gScore;
    std::unordered_map<int64_t, Pos>  cameFrom;

    auto key = [](Pos p) { return (int64_t)p.x << 32 | (uint32_t)p.y; };

    gScore[key(start)] = 0;
    open.push({start, 0, heuristic(start, goal)});

    const int dx[]    = {-1, 0, 1,-1, 1,-1, 0, 1};
    const int dy[]    = {-1,-1,-1, 0, 0, 1, 1, 1};
    const float cost[]= {1.414f,1,1.414f,1,1,1.414f,1,1.414f};

    int nodesExplored = 0;

    while (!open.empty()) {
        // Bail out if search is taking too long
        if (++nodesExplored > maxNodes) return {};

        Node cur = open.top(); open.pop();

        if (cur.pos.x == goal.x && cur.pos.y == goal.y) {
            std::vector<Pos> path;
            Pos p = goal;
            while (!(p.x == start.x && p.y == start.y)) {
                path.push_back(p);
                p = cameFrom[key(p)];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (int i = 0; i < 8; i++) {
            Pos nb{cur.pos.x + dx[i], cur.pos.y + dy[i]};
            Cell c = checkCell(world, nb);
            if (skipChecks == 0.0f && (c.bg.column == Terrain::Water || c.fg.state)) continue;

            float tentG = gScore[key(cur.pos)] + cost[i];
            if (!gScore.count(key(nb)) || tentG < gScore[key(nb)]) {
                gScore[key(nb)] = tentG;
                cameFrom[key(nb)] = cur.pos;
                open.push({nb, tentG, tentG + heuristic(nb, goal)});
            }
        }
    }
    return {};
}
