#include "helper.hpp"
#include "types.hpp"

int64_t getKey(int cx, int cy) {
	return (int64_t(cx) << 32) | int64_t(uint32_t(cy));
}

Chunk makeChunk(){
	Chunk c;
	c.tex = nullptr;
	return c;
}

void populateChunks(std::unordered_map<int64_t, Chunk>& world,int chunksX, int chunksY,int s) {
	for (int cy = 0; cy < chunksY; cy++)
		for (int cx = 0; cx < chunksX; cx++) {
			int64_t key = getKey(cx, cy);
			if (world.find(key) == world.end())
				world[key] = makeChunk();
		}
}

SDL_Texture* chunkTex(SDL_Renderer* renderer, Chunk& chunk, int bakeSize, SDL_Texture* atlas) {
	SDL_Texture* tex = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,
			stride * chunkW,
			stride * chunkH);
	if (!tex) return nullptr;
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

	SDL_SetRenderTarget(renderer, tex);
	SDL_SetRenderDrawColor(renderer,bg.r, bg.g, bg.b, bg.a);
	SDL_RenderClear(renderer);

	for (int i = 0; i < chunkW * chunkH; i++) {
		int cx = i % chunkW;
		int cy = i / chunkW;
		SDL_FRect dst = {(float)(cx * bakeSize),(float)(cy * bakeSize),
			(float)bakeSize,(float)bakeSize};

		if (!chunk.c[i].bg.state){
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &dst);
		}else{
			SDL_FRect src = {(float)(chunk.c[i].bg.row*ogBakeSize),
				(float)(chunk.c[i].bg.column*ogBakeSize),
				(float)ogBakeSize,(float)ogBakeSize};
			SDL_RenderTexture(renderer, atlas, &src, &dst);
		}

		if (chunk.c[i].fg.state){
			SDL_FRect src = {(float)(chunk.c[i].fg.row*ogBakeSize),
				(float)(chunk.c[i].fg.column*ogBakeSize),
				(float)ogBakeSize,(float)ogBakeSize};
			SDL_RenderTexture(renderer, atlas, &src, &dst);
		}
	}

	SDL_SetRenderTarget(renderer, nullptr);
	SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
	return tex;
}

void editTex(SDL_Renderer* renderer, Chunk& chunk,int bakeSize, SDL_Texture* atlas) {
	if (!chunk.tex) return;

	SDL_SetRenderTarget(renderer, chunk.tex);

	for (auto& cell : chunk.cells) {
		SDL_FRect dst = {(float)(cell.x*bakeSize), (float)(cell.y*bakeSize),
			(float)bakeSize, (float)bakeSize};


		if (!cell.c.bg.state){
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &dst);
		}else{
			SDL_FRect src = {(float)(cell.c.bg.row*ogBakeSize), (float)(cell.c.bg.column*ogBakeSize),
				(float)ogBakeSize,(float)ogBakeSize};
			SDL_RenderTexture(renderer, atlas, &src, &dst);
		}

		if (cell.c.fg.state){
			SDL_FRect src = {(float)(cell.c.fg.row*ogBakeSize), (float)(cell.c.fg.column*ogBakeSize),
				(float)ogBakeSize,(float)ogBakeSize};
			SDL_RenderTexture(renderer, atlas, &src, &dst);
		}


		int idx = cell.y * chunkW + cell.x;
		chunk.c[idx] = cell.c;
	}
	SDL_SetRenderTarget(renderer, nullptr);
}

void drawFPS(SDL_Renderer* renderer, GameFont& font, float fps, int x, int y) {
    std::string text = "FPS: " + std::to_string((int)fps);
    if (text != font.lastFpsText) {
        if (font.fpsTex) SDL_DestroyTexture(font.fpsTex);
        SDL_Surface* surf = TTF_RenderText_Blended(font.ttf, text.c_str(), 0, {0, 0, 0, 255});
        if (!surf) return;
        font.fpsTex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);
        font.lastFpsText = text;
    }
    if (!font.fpsTex) return;
    float w = 0, h = 0;
    SDL_GetTextureSize(font.fpsTex, &w, &h);
    const float scale = 0.4f;
    SDL_FRect dst = {(float)x, (float)y, w * scale, h * scale};
    SDL_RenderTexture(renderer, font.fpsTex, nullptr, &dst);
}

ChunkCoord toChunk(int x, int y){
	return { x/chunkW, y/chunkH, x%chunkW, y%chunkH, (y%chunkH)*chunkW+(x%chunkW)};
}


void updateVisibleChunks(std::unordered_map<int64_t, Chunk>& world, SDL_Renderer* renderer,
                        SDL_Texture* atlas, int minCX, int maxCX, int minCY, int maxCY, int bakeSize) {

    for (int cy = minCY; cy <= maxCY; cy++) {
        for (int cx = minCX; cx <= maxCX; cx++) {
            int64_t key = getKey(cx, cy);
            auto it = world.find(key);
            if (it != world.end()) {
                Chunk& chunk = it->second;
                if (!chunk.tex) {
                    chunk.tex = chunkTex(renderer, chunk, bakeSize, atlas);
                }
            }
        }
    }

    std::vector<int64_t> allKeys;
    allKeys.reserve(world.size());
    for (auto const& [key, _] : world) {
        allKeys.push_back(key);
    }

    const int unloadMargin = 3;
    for (int64_t key : allKeys) {
        // Re-verify the key exists because pathfinding might have deleted it
        // (though unlikely in your current setup, it's safe practice)
        auto it = world.find(key);
        if (it == world.end()) continue;

        int cx = (int)(key >> 32);
        int cy = (int)(key & 0xFFFFFFFF); // Use bitmask for safety

        if (cx < minCX - unloadMargin || cx > maxCX + unloadMargin ||
            cy < minCY - unloadMargin || cy > maxCY + unloadMargin) {

            Chunk& chunk = it->second;
            if (chunk.tex) {
                SDL_DestroyTexture(chunk.tex);
                chunk.tex = nullptr;
            }
        }
    }
}

void changeCell(std::unordered_map<int64_t, Chunk>& world, Pos p, Cell c, bool restore){
    ChunkCoord coords = toChunk(p.x, p.y);
    int64_t key = getKey(coords.cx, coords.cy);

    auto it = world.find(key);
    if (it == world.end()) return;

    Chunk& chunk = it->second;
    EditCell cell;
    cell.x = coords.lx;
    cell.y = coords.ly;
    cell.c = restore ? chunk.ogC[coords.ly * chunkW + coords.lx] : c;
    chunk.cells.push_back(cell);
}

Cell checkCell(std::unordered_map<int64_t, Chunk>& world, Pos p){
	ChunkCoord coords = toChunk(p.x, p.y);
	auto it = world.find(getKey(coords.cx, coords.cy));
	if (it == world.end()) return Cell{}; // sentinel
	return it->second.c[coords.ly * chunkW + coords.lx];
}

float clampFloat(float f, float l, float h){
	if (f < l) f = l;
	if (f > h) f = h;

	return f;
}
