#include "pathFinding.hpp"
#include "types.hpp"

static inline float heuristic(Pos a, Pos b) {
	float adx = (float)std::abs(a.x - b.x);
	float ady = (float)std::abs(a.y - b.y);
	return (adx > ady) ? 0.414f * ady + adx : 0.414f * adx + ady;
}

std::vector<Pos> astar(Pos start, Pos goal,std::unordered_map<int64_t, Chunk>& world,
		float skipChecks, int maxNodes){
	if (start.x == goal.x && start.y == goal.y) return {};

	struct Info { float g; Pos parent; };
	std::unordered_map<int64_t, Info> visited;
	visited.reserve(512);

	std::vector<Node> openBuf;
	openBuf.reserve(512);
	std::priority_queue<Node, std::vector<Node>, std::greater<Node>>
		open(std::greater<Node>{}, std::move(openBuf));

	const int64_t startKey = getKey(start);
	const int64_t goalKey  = getKey(goal);

	visited[startKey] = { 0.0f, start };
	open.push({ start, 0.0f, heuristic(start, goal) });

	constexpr int   DX[]   = { -1, 0, 1,-1, 1,-1, 0, 1 };
	constexpr int   DY[]   = { -1,-1,-1, 0, 0, 1, 1, 1 };
	constexpr float COST[] = { 1.414f,1,1.414f,1,1,1.414f,1,1.414f };
	const bool doChecks = (skipChecks == 0.0f);
	int nodesExplored = 0;

	while (!open.empty()) {
		if (++nodesExplored > maxNodes) return {};

		Node cur = open.top(); open.pop();

		const int64_t curKey = getKey(cur.pos);
		auto curIt = visited.find(curKey);

		if (curIt == visited.end() || cur.g > curIt->second.g) continue;

		const float curG = curIt->second.g;

		for (int i = 0; i < 8; ++i) {
			Pos nb{ cur.pos.x + DX[i], cur.pos.y + DY[i] };

			if (doChecks) {
				Cell* c = checkCell(world, nb);
				if (!c || !walkable(c)) continue;
			}

			const int64_t nbKey = getKey(nb);
			const float   tentG = curG + COST[i];

			auto [nbIt, inserted] = visited.emplace(nbKey, Info{ tentG, cur.pos });
			if (!inserted) {
				if (tentG >= nbIt->second.g) continue;
				nbIt->second = { tentG, cur.pos };
			}

			if (nbKey == goalKey) {
				std::vector<Pos> path;
				path.reserve(64);

				Pos p = nb;
				int64_t k = nbKey;
				int safetyCounter = 0;

				while (k != startKey && safetyCounter++ < maxNodes) {
					path.push_back(p);
					auto it = visited.find(k);
					if (it == visited.end()) break; // Break out if graph link is broken
					p = it->second.parent;
					k = getKey(p);
				}
				return path;
			}
			open.push({ nb, tentG, tentG + heuristic(nb, goal) });
		}
	}
	return {};
}
