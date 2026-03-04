#include "levelGeneration.hpp"
#include <random>
#include "FastNoiseLite.h"
#include <unordered_map>
#include <cstdint>
#include "helper.hpp"


FastNoiseLite noise;
int levelSizeX = 2000; int levelSizeY=2000;

void generateLevel(std::unordered_map<int64_t, Chunk>& world, int x, int y){
	static std::mt19937 rng(std::random_device{}());
	static std::uniform_real_distribution<float> dist(1.0f, 100000.0f);
	float varX = dist(rng);
	float varY = dist(rng);

	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

	for (size_t y=0; y<levelSizeY; y++){
		for (size_t x=0; x<levelSizeX; x++){
			int cx = x/chunkW;
			int cy = y/chunkH;

			int64_t key = getKey(cx, cy);	

			float value = noise.GetNoise((x+varX), (y+varY));

			char shade;
			Color color=BLACK;
			if (value >= 0.9f){      shade = '@';color=WHITE;}
			else if (value >= 0.8f){ shade = '#';color=GRAY;}
			else if (value >= 0.6f){ shade = '=';color=DARKGREEN;}
			else if (value >= 0.4f){ shade = '-';color=LIME;}
			else if (value >= 0.2f){ shade = '#';color=BEIGE;}
			else                    {shade = '@';color=DARKBLUE;}

			int lx = x - cx * chunkW;
			int ly = y - cy * chunkH;
			int idx = ly * chunkW + lx;

			Chunk& chunk = world[key];
			chunk.codepoints[idx]=shade;
			chunk.colors[idx]=color;
		}
	}
}
