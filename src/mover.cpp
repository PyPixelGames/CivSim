#include "mover.hpp"
#include "types.hpp"
#include <unordered_map>

void Mover::update(std::unordered_map<int64_t, Chunk>& world){
	x++;
	int cx = x/chunkW;
	int cy = y/chunkH;

	int lx = x-(cx*chunkW);
	int ly = y-(cy*chunkH);

	Chunk& chunk = world[getKey(cx, cy)];

	EditCell cell;
	cell.x = lx;
	cell.y = ly;
	cell.c = Colors::PURPLE;
	cell.ch = '!';
	chunk.cells.push_back(cell);
}
