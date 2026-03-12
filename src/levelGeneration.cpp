#include "levelGeneration.hpp"
#include "helper.hpp"
#include "FastNoiseLite.h"
#include "types.hpp"
#include <random>
#include <unordered_set>

using namespace Colors;   // WHITE, GRAY, DARKGREEN, etc.

static int levelSizeX = 640;
static int levelSizeY = 640;
static std::mt19937 rng(std::random_device{}());

void generateLevel(std::unordered_map<int64_t, Chunk>& world,SDL_Renderer* renderer,int s) {
	static std::uniform_real_distribution<float> dist(1.0f, 100000.0f);
	float varX = dist(rng);
	float varY = dist(rng);

	static FastNoiseLite noise;
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise.SetFrequency(0.005f);
	noise.SetFractalType(FastNoiseLite::FractalType_FBm);
	noise.SetFractalOctaves(5);
	noise.SetFractalLacunarity(2.0f);
	noise.SetFractalGain(0.5f);

	static FastNoiseLite vegNoise;
	vegNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	vegNoise.SetFrequency(0.008f);
	vegNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
	vegNoise.SetFractalOctaves(3);
	float vegVarX = dist(rng);
	float vegVarY = dist(rng);

	std::unordered_set<int64_t> dirtyKeys;

	for (int ty = 0; ty < levelSizeY; ty++) {
		for (int tx = 0; tx < levelSizeX; tx++) {
			int cx = tx / chunkW;
			int cy = ty / chunkH;
			int64_t key = getKey(cx, cy);
			if (world.find(key) == world.end()) continue;

			float value = noise.GetNoise((float)(tx + varX), (float)(ty + varY));
			value = (value + 1.0f) / 2.0f;

			char      shade;
			SDL_Color color;

			if      (value >= 0.85f) {shade = '^'; color = WHITE;}      // peaks
			else if (value >= 0.78f) {shade = '+'; color = DARKGRAY;}   // mountains
			else if (value >= 0.37f) {shade = '.'; color = LIME;}       // fields
			else if (value >= 0.33f) {shade = '.'; color = YELLOW;}     // sand/beach (wider now)
			else                     {shade = '~'; color = DARKBLUE;}   // water
			
			float vegValue = vegNoise.GetNoise((float)(tx+vegVarX), (float)(ty+vegVarY));
			vegValue = (vegValue + 1.0f) / 2.0f;
			if (vegValue >= 0.70f && shade == '.'){
				shade='T';
				color=DARKGREEN;
			}

			Chunk& chunk  = world[key];
			int localX = tx % chunkW;
			int localY = ty % chunkH;
			int index  = localY * chunkW + localX;

			chunk.codepoints[index] = shade;
			chunk.ogCodepoints[index]=shade;
			chunk.colors[index] = color;
			chunk.ogColors[index] = color;
			dirtyKeys.insert(key);
		}
	}

	// Re-bake every chunk that changed
	for (int64_t key : dirtyKeys) {
		Chunk& chunk = world[key];
		if (chunk.tex) SDL_DestroyTexture(chunk.tex);
		chunk.tex = chunkTex(renderer, chunk, s);
	}
}
