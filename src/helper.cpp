#include "helper.hpp"
#include <raylib.h>
#include <rlgl.h>

int64_t getKey(int cx, int cy) {
	return (int64_t(cx) << 32) | int64_t(uint32_t(cy));
}

Chunk makeChunk(Font font,int s) {
	Chunk c;
	for (int i = 0; i < chunkW * chunkH; i++) {
		c.codepoints[i] = '`';
		c.colors[i] = Color{52, 52, 52, 255};
	}
	//c.tex = chunkTex(c, font, s);
	return c;
}

std::pair<int, int> resize(int cellSize, Font font){
	int glyphIndex = GetGlyphIndex(font, 'A');
	int advance = font.glyphs[glyphIndex].advanceX;

	float scale = (float)cellSize / font.baseSize;

	int gap = 4; //increase this value to decrease the gap size

	int cellW = (int)((advance - font.glyphs[GetGlyphIndex(font,'A')].offsetX)*scale)-gap;
	int cellH = (int)((font.baseSize - font.glyphs[GetGlyphIndex(font,'A')].offsetY) * scale)-gap;

	return {cellW, cellH};
}

void populateChunks(std::unordered_map<int64_t, Chunk>& world,int chunksX,int chunksY,Font font,int s){
	for (int cy = 0; cy < chunksY; cy++) {
		for (int cx = 0; cx < chunksX; cx++) {
			int64_t key = getKey(cx, cy);
			if (world.find(key) == world.end()) {
				world[key] = makeChunk(font, s);
			}
		}
	}
}

std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> parts;
	std::stringstream ss(s);
	std::string item;

	while (std::getline(ss, item, delim)) {
		parts.push_back(item);
	}
	return parts;
}


RenderTexture2D chunkTex(Chunk& chunk, Font font, int bakeSize){
	RenderTexture2D tex = LoadRenderTexture(bakeSize * chunkW, bakeSize * chunkH);

	BeginTextureMode(tex);
	rlViewport(0, 0, tex.texture.width, tex.texture.height);
	ClearBackground(BLACK);

	for (size_t i = 0; i < std::size(chunk.codepoints); i++){
		int x = i % chunkW;
		int y = i / chunkW;
		DrawTextCodepoint(font, chunk.codepoints[i],
			{(float)x * bakeSize, (float)y * bakeSize},
			bakeSize,
		chunk.colors[i]);
	}
	EndTextureMode();
	return tex;
}

RenderTexture2D editTex(Chunk& chunk, Font font, int bakeSize){
	BeginTextureMode(chunk.tex);

	for (auto cell : chunk.cells){
		DrawRectangle(cell.x*bakeSize, cell.y*bakeSize, bakeSize, bakeSize, BLACK);	
		DrawTextCodepoint(font, cell.ch,
			{(float)cell.x * bakeSize, (float)cell.y * bakeSize},bakeSize, cell.c);
		int	idx=cell.y*chunkW+cell.x;
		chunk.codepoints[idx] = cell.ch;
		chunk.colors[idx] = cell.c;
	}

	chunk.cells.clear();

	EndTextureMode();

	return chunk.tex;
}
