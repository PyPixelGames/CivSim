#include "creature.hpp"
#include "types.hpp"
#include <unordered_map>
#include <iostream>

void Creature::update(std::unordered_map<int64_t, Chunk>& world,
		std::vector<Creature*> /*creatures*/){
	if (!alive){changeCell(world, pos, Cell{}, true); return;} //restore
	if (path.size()>0){
		Pos nextPos = path.front();
		updatePosition(world, nextPos);
		path.erase(path.begin());
	}
	updateMood(world);
}

void Creature::updatePosition(std::unordered_map<int64_t, Chunk>& world,Pos newPos){
	Cell p = checkCell(world, newPos);
	cell.bg = p.bg;
	changeCell(world, pos, Cell{}, true);  // restore
	pos = newPos;
	changeCell(world, pos, cell, false); // place
}

void Creature::updateMood(std::unordered_map<int64_t, Chunk>& world){
	meal.food-=0.01;

	mood.happiness += meal.evaluate()*dna.find("foodLove")->value;

	mood.clamp();
	meal.clamp();

	if (meal.food <= 1){
		std::cout << "FIND FOOD" << std::endl;
		Cell bushCell{};
		bushCell.fg.column = bush;
		bushCell.fg.row = -1;
		bushCell.fg.state=true;

		bushCell.bg.state=true;


		Pos bushPos = lookFor(world, bushCell);
		if (bushPos.x!=-1 && bushPos.y!=-1){
			std::cout << "Found bush: " << bushPos.x << ", " << bushPos.y << std::endl;
		}else{
			std::cout << "No bush found" << std::endl;
		}
	}
	if (meal.food<=-meal.val){
		alive=false;
		std::cout << "dead" << std::endl;
	}
}

void Creature::pathFind(std::unordered_map<int64_t, Chunk>& world,Pos targetPos){
	if (targetPos.x == -1 && targetPos.y == -1){
		bool search = true;
		while (search){
			goal.x = RandomPos(rng);
			goal.y = RandomPos(rng);
			Cell c = checkCell(world, goal);
			if (c.bg.column != Water && c.fg.state!=true){
				break;
			}
		}
	}else{
		goal=targetPos;
	}
	path = astar(this->pos, goal, world);
}

Pos Creature::lookFor(std::unordered_map<int64_t, Chunk>& world,Cell target){
	Pos closest = {-1, -1};

	std::unordered_set<int64_t> visibleChunks;
	int sight = dna.find("sight")->value;

	for(int y=this->pos.y-sight; y <= this->pos.y+sight; y+=chunkH/2){
		for(int x=this->pos.x-sight; x <= this->pos.x+sight; x+=chunkW/2){
			if (x<0 || y<0) continue;

			ChunkCoord coords = toChunk(x, y);
			int64_t key = getKey(coords.cx, coords.cy);
			if (world.find(key) == world.end()) continue;

			visibleChunks.insert(key);
		}
	}

	int bestDist = sight;

	std::vector<Pos> possiblePositions;
	for (const auto& key: visibleChunks){
		Chunk& chunk = world[key];
		if (chunk.find(target)){
			for (int y=0; y<chunkH; y++){
				for (int x=0; x<chunkW; x++){
					Cell c = chunk.c[y * chunkW + x];
					if (c.mask(target)){
						auto [cx, cy] = fromKey(key);
						Pos p = {(cx * chunkW) + x, (cy * chunkH) + y};
						int dist = this->pos.distance(p);
						if (dist < bestDist) {
							bestDist = dist;
							closest = p;
						}
					}
				}
			}
		}
	}

	return closest;
}
