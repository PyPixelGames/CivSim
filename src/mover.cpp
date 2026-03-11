#include "mover.hpp"
#include "types.hpp"
#include <unordered_map>

void Mover::update(std::unordered_map<int64_t, Chunk>& world){
	if (checkCell(world, x+1,y) != '~'){
		changeCell(world, x, y, 'g', Colors::PURPLE); // 'g' is to return the og character
		x++;
		changeCell(world, x, y, '!', Colors::PURPLE);
	}
}

void Mover::changeCell(std::unordered_map<int64_t, Chunk>& world, int x, int y,
		char c, SDL_Color color){
	ChunkCoord coords = toChunk(x, y);
	Chunk& chunk = world[getKey(coords.cx, coords.cy)];

	EditCell cell;
	cell.x = coords.lx;
	cell.y = coords.ly;


	if (c!='g'){
		cell.ch=c;
		cell.c = color;
	}else{
		cell.ch=chunk.ogCodepoints[coords.ly*chunkW+coords.lx];
		cell.c = chunk.ogColors[coords.ly*chunkW+coords.lx];
	}

	chunk.cells.push_back(cell);	
}

char Mover::checkCell(std::unordered_map<int64_t, Chunk>& world, int x, int y){
    ChunkCoord coords = toChunk(x, y);
    auto it = world.find(getKey(coords.cx, coords.cy));
    if (it == world.end()) return '`'; // or whatever your "empty" sentinel is
    return it->second.codepoints[coords.ly*chunkW+coords.lx];
}

