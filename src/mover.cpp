#include "mover.hpp"
#include "types.hpp"
#include <unordered_map>

void Mover::update(std::unordered_map<int64_t, Chunk>& world){
	Cell p = checkCell(world, x+1, y);
	if (p.bg.y != Terrain::Water){
		changeCell(world, x, y, {-1, -1}, true);  // restore
		x++;
		changeCell(world, x, y, {-1, -1}, false); // place
	}
}

void Mover::changeCell(std::unordered_map<int64_t, Chunk>& world,int x,int y,Cell c,bool restore){
	ChunkCoord coords = toChunk(x, y);
	Chunk& chunk = world[getKey(coords.cx, coords.cy)];
	EditCell cell;
	cell.x = coords.lx;
	cell.y = coords.ly;
	if (restore){
		cell.c = chunk.ogC[coords.ly * chunkW + coords.lx];
	} else {
		cell.c = c;
	}
	chunk.cells.push_back(cell);
}

Cell Mover::checkCell(std::unordered_map<int64_t, Chunk>& world, int x, int y){
	ChunkCoord coords = toChunk(x, y);
	auto it = world.find(getKey(coords.cx, coords.cy));
	if (it == world.end()) return Cell{}; // sentinel
	return it->second.c[coords.ly * chunkW + coords.lx];
}
