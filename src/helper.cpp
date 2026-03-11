#include "helper.hpp"
#include <sstream>

int64_t getKey(int cx, int cy) {
	return (int64_t(cx) << 32) | int64_t(uint32_t(cy));
}

Chunk makeChunk(GameFont& /*font*/, int /*s*/) {
	Chunk c;
	for (int i = 0; i < chunkW * chunkH; i++) {
		c.codepoints[i] = '`';
		c.colors[i]     = {52, 52, 52, 255};
	}
	c.tex = nullptr;
	return c;
}

void populateChunks(std::unordered_map<int64_t, Chunk>& world,int chunksX, int chunksY,GameFont& font,
		int s) {
	for (int cy = 0; cy < chunksY; cy++)
		for (int cx = 0; cx < chunksX; cx++) {
			int64_t key = getKey(cx, cy);
			if (world.find(key) == world.end())
				world[key] = makeChunk(font, s);
		}
}

void drawGlyph(SDL_Renderer* renderer, GameFont& font,int codepoint, float x, float y, int cellSize,
		SDL_Color color) {
	auto it = font.glyphCache.find(codepoint);
	SDL_Texture* tex = nullptr;

	if (it == font.glyphCache.end()) {
		SDL_Color white = {255, 255, 255, 255};
		SDL_Surface* surf = TTF_RenderGlyph_Blended(font.ttf,(Uint32)codepoint,white);
		if (!surf) return;

		tex = SDL_CreateTextureFromSurface(renderer, surf);
		SDL_DestroySurface(surf);
		if (!tex) return;

		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
		font.glyphCache[codepoint] = tex;
	} else {
		tex = it->second;
	}

	SDL_SetTextureColorMod(tex, color.r, color.g, color.b);
	SDL_SetTextureAlphaMod(tex, color.a);

	SDL_FRect dst = {x, y, (float)cellSize, (float)cellSize};
	SDL_RenderTexture(renderer, tex, nullptr, &dst);
}

SDL_Texture* chunkTex(SDL_Renderer* renderer, Chunk& chunk,GameFont& font, int bakeSize) {
	SDL_Texture* tex = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_TARGET,
			bakeSize * chunkW,
			bakeSize * chunkH);
	if (!tex) return nullptr;
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

	SDL_SetRenderTarget(renderer, tex);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	for (int i = 0; i < chunkW * chunkH; i++) {
		int cx = i % chunkW;
		int cy = i / chunkW;
		drawGlyph(renderer, font,
				chunk.codepoints[i],
				(float)(cx * bakeSize),
				(float)(cy * bakeSize),
				bakeSize,
				chunk.colors[i]);
	}

	SDL_SetRenderTarget(renderer, nullptr);
	return tex;
}

void editTex(SDL_Renderer* renderer, Chunk& chunk,
		GameFont& font, int bakeSize) {
	if (!chunk.tex) return;

	SDL_SetRenderTarget(renderer, chunk.tex);

	for (auto& cell : chunk.cells) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_FRect rect = {
			(float)(cell.x * bakeSize),
			(float)(cell.y * bakeSize),
			(float)bakeSize,
			(float)bakeSize
		};
		SDL_RenderFillRect(renderer, &rect);

		drawGlyph(renderer, font,
				cell.ch,
				(float)(cell.x * bakeSize),
				(float)(cell.y * bakeSize),
				bakeSize,
				cell.c);

		int idx = cell.y * chunkW + cell.x;
		chunk.codepoints[idx] = cell.ch;
		chunk.colors[idx]     = cell.c;
	}

	SDL_SetRenderTarget(renderer, nullptr);
}

void drawFPS(SDL_Renderer* renderer, GameFont& font,
		float fps, int x, int y) {
	std::string text = "FPS: " + std::to_string((int)fps);
	SDL_Color color  = {0, 255, 40, 255};

	SDL_Surface* surf = TTF_RenderText_Blended(font.ttf,
			text.c_str(), 0,
			color);
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
