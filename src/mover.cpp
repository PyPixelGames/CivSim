#include "mover.hpp"
#include "types.hpp"
#include <unordered_map>
#include <iostream>

void Mover::update(std::unordered_map<int64_t, Chunk>& world){
	if (path.size()>0){
		Pos nextPos = path.front();
		Cell p = checkCell(world, nextPos);
		changeCell(world, pos, {-1, -1}, true);  // restore
		pos = nextPos;
		changeCell(world, pos, {-1, -1}, false); // place
		path.erase(path.begin());
	}
}
