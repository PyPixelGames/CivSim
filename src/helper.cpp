#include "helper.hpp"
#include <sstream>

int64_t getKey(int cx, int cy) {
	return (int64_t(cx) << 32) | int64_t(uint32_t(cy));
}

Chunk makeChunk(){
	Chunk c;
	c.tex = nullptr;
	return c;
}

void populateChunks(std::unordered_map<int64_t, Chunk>& world,int chunksX, int chunksY,
		int s) {
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
			SDL_FRect src = {(float)(chunk.c[i].bg.x*bakeSize), (float)(chunk.c[i].bg.y*bakeSize),
				(float)bakeSize,(float)bakeSize};
			SDL_RenderTexture(renderer, atlas, &src, &dst); 
		}

		if (chunk.c[i].fg.state){
			SDL_FRect src = {(float)(chunk.c[i].fg.x*bakeSize), (float)(chunk.c[i].fg.y*bakeSize),
				(float)bakeSize,(float)bakeSize};
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
		SDL_FRect dst = {(float)(cell.x*bakeSize), (float)(cell.y*bakeSize), (float)bakeSize, (float)bakeSize};


		if (!cell.c.bg.state){
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &dst);
		}else{
			SDL_FRect src = {(float)(cell.c.bg.x*bakeSize), (float)(cell.c.bg.y*bakeSize),
				(float)bakeSize,(float)bakeSize};
			SDL_RenderTexture(renderer, atlas, &src, &dst); 
		}

		if (cell.c.fg.state){
			SDL_FRect src = {(float)(cell.c.fg.x*bakeSize), (float)(cell.c.fg.y*bakeSize),
				(float)bakeSize,(float)bakeSize};
			SDL_RenderTexture(renderer, atlas, &src, &dst);	
		}


		int idx = cell.y * chunkW + cell.x;
		chunk.c[idx] = cell.c;
	}

	SDL_SetRenderTarget(renderer, nullptr);
	SDL_SetTextureScaleMode(chunk.tex, SDL_SCALEMODE_NEAREST);
}

void drawFPS(SDL_Renderer* renderer, GameFont& font,
		float fps, int x, int y) {
	std::string text = "FPS: " + std::to_string((int)fps);
	SDL_Color color  = Colors::BLACK;

	SDL_Surface* surf = TTF_RenderText_Blended(font.ttf,text.c_str(), 0,color);
	if (!surf) return;

	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_DestroySurface(surf);
	if (!tex) return;

	float w = 0, h = 0;
	SDL_GetTextureSize(tex, &w, &h);
	const float scale = 0.4f;
	SDL_FRect dst = {(float)x, (float)y, w * scale, h * scale};
	SDL_RenderTexture(renderer, tex, nullptr, &dst);
	SDL_DestroyTexture(tex);
}

ChunkCoord toChunk(int x, int y){
	return { x/chunkW, y/chunkH, x%chunkW, y%chunkH };
}
