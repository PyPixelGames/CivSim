#include "mover.hpp"
#include "types.hpp"
#include <unordered_map>

void Mover::update(std::unordered_map<int64_t, Chunk>& world){
	changeCell(world, {x, y}, 'g', Colors::PURPLE); // 'g' is to return the og character
	x++;
	changeCell(world, {x, y}, '!', Colors::PURPLE);
}

void Mover::changeCell(std::unordered_map<int64_t, Chunk>& world, std::pair<int, int> pos,
		char c, SDL_Color color){
	int cx = pos.first/chunkW;
	int cy = pos.second/chunkH;

	int lx = pos.first-(cx*chunkW);
	int ly = pos.second-(cy*chunkH);

	Chunk& chunk = world[getKey(cx, cy)];

	EditCell cell;
	cell.x = lx;
	cell.y = ly;


	if (c!='g'){
		cell.ch=c;
		cell.c = color;
	}else{
		cell.ch=chunk.ogCodepoints[ly*chunkW+lx];
		cell.c = chunk.ogColors[ly*chunkW+lx];
	}

	chunk.cells.push_back(cell);	
}
