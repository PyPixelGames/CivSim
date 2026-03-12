#include "helper.hpp"
#include <sstream>

int64_t getKey(int cx, int cy) {
	return (int64_t(cx) << 32) | int64_t(uint32_t(cy));
}

Chunk makeChunk(int /*s*/) {
	Chunk c;
	for (int i = 0; i < chunkW * chunkH; i++) {
		c.colors[i] = {52, 52, 52, 255};
		c.ogColors[i] = {52, 52, 52, 255};
	}
	c.tex = nullptr;
	return c;
}

void populateChunks(std::unordered_map<int64_t, Chunk>& world,int chunksX, int chunksY,
		int s) {
	for (int cy = 0; cy < chunksY; cy++)
		for (int cx = 0; cx < chunksX; cx++) {
			int64_t key = getKey(cx, cy);
			if (world.find(key) == world.end())
				world[key] = makeChunk(s);
		}
}

SDL_Texture* chunkTex(SDL_Renderer* renderer, Chunk& chunk, int bakeSize) {
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
		SDL_SetRenderDrawColor(renderer, chunk.colors[i].r, chunk.colors[i].g, 
				chunk.colors[i].b, chunk.colors[i].a);
		SDL_FRect rect = {(float)(cx * bakeSize),(float)(cy * bakeSize),
			(float)bakeSize+2,(float)bakeSize+2};
		SDL_RenderFillRect(renderer, &rect);
	}

	SDL_SetRenderTarget(renderer, nullptr);
	SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
	return tex;
}

void editTex(SDL_Renderer* renderer, Chunk& chunk,int bakeSize) {
	if (!chunk.tex) return;

	SDL_SetRenderTarget(renderer, chunk.tex);

	for (auto& cell : chunk.cells) {
		SDL_SetRenderDrawColor(renderer, cell.c.r, cell.c.g, cell.c.b, cell.c.a);
		SDL_FRect rect = {
			(float)(cell.x * bakeSize),
			(float)(cell.y * bakeSize),
			(float)bakeSize+2,
			(float)bakeSize+2};
		SDL_RenderFillRect(renderer, &rect);	

		int idx = cell.y * chunkW + cell.x;
		chunk.colors[idx]     = cell.c;
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

bool compareColors(SDL_Color c1, SDL_Color c2){
	return (c1.a==c2.a && c1.r==c2.r && c1.g==c2.g && c1.b==c2.b);
}
