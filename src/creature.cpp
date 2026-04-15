#include "creature.hpp"
#include "types.hpp"
#include <unordered_map>
#include <iostream>

void Creature::update(std::unordered_map<int64_t, Chunk>& world){
	if (!alive) return;
	if (path.size()>0){
		Pos nextPos = path.front();
		Cell p = checkCell(world, nextPos);
		changeCell(world, pos, {-1, -1}, true);  // restore
		pos = nextPos;
		changeCell(world, pos, {-1, -1}, false); // place
		path.erase(path.begin());
	}

	updateMood();
}

void Creature::updateMood(){
	mood.happiness += meal.evaluate()*FoodLove;

	mood.clamp();
	meal.clamp();

	if (meal.food<=-meal.val){
		alive=false;
		std::cout << "dead" << std::endl;
	}
}
