#include "creature.hpp"
#include "types.hpp"
#include <unordered_map>
#include <iostream>

void Creature::update(std::unordered_map<int64_t, Chunk>& world,std::vector<Creature*> creatures){
	if (!alive){changeCell(world, pos, {-1, -1}, true); return;} //restore
	if (path.size()>0){
		Pos nextPos = path.front();
		updatePosition(world, nextPos);
		path.erase(path.begin());
	}
	updateMood();
}

void Creature::updatePosition(std::unordered_map<int64_t, Chunk>& world,Pos newPos){
	Cell p = checkCell(world, newPos);
	cell.bg = p.bg;
	changeCell(world, pos, {-1, -1}, true);  // restore
	pos = newPos;
	changeCell(world, pos, cell, false); // place
}

void Creature::updateMood(){
	meal.food-=0.01;

	mood.happiness += meal.evaluate()*dna.find("foodLove")->value;

	mood.clamp();
	meal.clamp();

	std::cout << id << ":  " << meal.food << std::endl;

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
			if (world.find(key) == world.end()) return closest;

			visibleChunks.insert(key);
		}
	}


	//Chunk& chunk = world[key];

	return closest;
}
