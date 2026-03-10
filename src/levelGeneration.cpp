#include "levelGeneration.hpp"
#include "helper.hpp"
#include "FastNoiseLite.h"
#include <random>
#include <unordered_set>

using namespace Colors;   // WHITE, GRAY, DARKGREEN, etc.

static FastNoiseLite noise;
static int levelSizeX = 320;
static int levelSizeY = 320;
static std::mt19937 rng(std::random_device{}());

void generateLevel(std::unordered_map<int64_t, Chunk>& world,
		int /*x*/, int /*y*/,
		SDL_Renderer* renderer,
		GameFont& font, int s) {
	static std::uniform_real_distribution<float> dist(1.0f, 100000.0f);
	float varX = dist(rng);
	float varY = dist(rng);

	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise.SetFrequency(0.05f);

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

			if      (value >= 0.85f) { shade = '@'; color = WHITE;     }
			else if (value >= 0.75f) { shade = '#'; color = GRAY;      }
			else if (value >= 0.60f) { shade = '='; color = DARKGREEN; }
			else if (value >= 0.45f) { shade = '-'; color = LIME;      }
			else if (value >= 0.35f) { shade = '.'; color = BEIGE;     }
			else                     { shade = '~'; color = DARKBLUE;  }

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
		chunk.tex = chunkTex(renderer, chunk, font, s);
	}
}
