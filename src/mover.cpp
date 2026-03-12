#include "mover.hpp"
#include "types.hpp"
#include <unordered_map>

void Mover::update(std::unordered_map<int64_t, Chunk>& world){
    if (! compareColors(checkCell(world, x+1, y), Colors::DARKBLUE)){
        changeCell(world, x, y, Colors::PURPLE, true);  // restore
        x++;
        changeCell(world, x, y, Colors::PURPLE, false); // place
    }
}

void Mover::changeCell(std::unordered_map<int64_t, Chunk>& world,int x,int y,SDL_Color color, bool restore){
    ChunkCoord coords = toChunk(x, y);
    Chunk& chunk = world[getKey(coords.cx, coords.cy)];
    EditCell cell;
    cell.x = coords.lx;
    cell.y = coords.ly;
    if (restore){
        cell.c = chunk.ogColors[coords.ly * chunkW + coords.lx];
    } else {
        cell.c = color;
    }
    chunk.cells.push_back(cell);
}

SDL_Color Mover::checkCell(std::unordered_map<int64_t, Chunk>& world, int x, int y){
    ChunkCoord coords = toChunk(x, y);
    auto it = world.find(getKey(coords.cx, coords.cy));
    if (it == world.end()) return Colors::BLACK; // sentinel
    return it->second.colors[coords.ly * chunkW + coords.lx];
}
