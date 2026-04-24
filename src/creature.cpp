#include "creature.hpp"
#include "types.hpp"
#include <unordered_map>
#include <iostream>

void Creature::update(std::unordered_map<int64_t, Chunk>& world,
		std::vector<Creature*> creatures){
	if (!alive) return;
	if (path.size()>0){
		Pos nextPos = path.front();
		Cell p = checkCell(world, nextPos);
		changeCell(world, pos, {-1, -1}, true);  // restore
		pos = nextPos;
		changeCell(world, pos, cell, false); // place
		path.erase(path.begin());
	}else{
		return;
	}

	for (auto crt : creatures){
		if (crt->id != id && id == 0){ //REMOVE id == 0, this is for testing
			int dis = pos.distance(crt->pos);
			if (dis < dna.find("sight")->value){
				std::cout << "Distance: " <<  dis << "   " <<
					" Id: " << crt->id << std::endl;
			}
		}
	}

	updateMood();
}

void Creature::updateMood(){
	mood.happiness += meal.evaluate()*dna.find("foodLove")->value;

	mood.clamp();
	meal.clamp();

	if (meal.food<=-meal.val){
		alive=false;
		std::cout << "dead" << std::endl;
	}
}
