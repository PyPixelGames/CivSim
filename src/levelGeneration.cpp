#include "levelGeneration.hpp"
#include "helper.hpp"
#include "FastNoiseLite.h"
#include "types.hpp"
#include <random>
#include <unordered_set>

static int levelSizeX = 640;
static int levelSizeY = 640;
static std::mt19937 rng(std::random_device{}());
static std::uniform_real_distribution<float> dist(1.0f, 100000.0f);

static std::uniform_real_distribution<float> biomeBlend(0.0f, 1.0f);

void generateLevel(std::unordered_map<int64_t, Chunk>& world,SDL_Renderer* renderer,int s, SDL_Texture* atlas) {	
	float varX = dist(rng);
	float varY = dist(rng);

	static FastNoiseLite terNoise;
	terNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	terNoise.SetFrequency(0.005f);
	terNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
	terNoise.SetFractalOctaves(5);
	terNoise.SetFractalLacunarity(2.0f);
	terNoise.SetFractalGain(0.5f);

	static FastNoiseLite vegNoise;
	vegNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	vegNoise.SetFrequency(0.008f);
	vegNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
	vegNoise.SetFractalOctaves(3);
	float vegVarX = dist(rng);
	float vegVarY = dist(rng);

	static FastNoiseLite biomeNoise;
	biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	biomeNoise.SetFrequency(0.0009f);
	biomeNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
	biomeNoise.SetFractalOctaves(3);
	float biomeVarX = dist(rng);
	float biomeVarY = dist(rng);

	std::unordered_set<int64_t> dirtyKeys;

	for (int ty = 0; ty < levelSizeY; ty++) {
		for (int tx = 0; tx < levelSizeX; tx++) {
			int cx = tx / chunkW;
			int cy = ty / chunkH;
			int64_t key = getKey(cx, cy);
			if (world.find(key) == world.end()) continue;
	
			Cell type{-1, -1};


			// Biome generation	
			float biomeValue = biomeNoise.GetNoise((float)(tx + biomeVarX),(float)(ty + biomeVarY));
			biomeValue = (biomeValue + 1.0f) / 2.0f; 
			
			if (biomeValue >= 0.505f){
				type.bg.x = SNOWY;
			}else if(biomeValue >= 0.495){
				float r = biomeBlend(rng);
				if (r>=0.5) {type.bg.x=FIELDS;}
				else      {type.bg.x=SNOWY;}

			}else{
				type.bg.x = FIELDS;
			}

			// Base terrain generation
			float terValue = terNoise.GetNoise((float)(tx + varX), (float)(ty + varY));
			terValue = (terValue + 1.0f) / 2.0f;

			if (terValue >= 0.85f){type.bg.y= MountainTop;}
			else if (terValue >= 0.78f) {type.bg.y= Mountain;}
			else if (terValue >= 0.37f) {type.bg.y= Grass;}
			else if (terValue >= 0.33f) {type.bg.y= Sand;}
			else                     {type.bg.y= Water;}
			type.bg.state=true;


			// Vegitation generation
			float vegValue = vegNoise.GetNoise((float)(tx+vegVarX), (float)(ty+vegVarY));
			vegValue = (vegValue + 1.0f) / 2.0f;
			if (vegValue >= 0.70f && type.bg.y== Grass && terValue<=0.5){
				type.fg.y= Tree;
				type.fg.x= type.bg.x;
				type.fg.state=true;
			}



			Chunk& chunk  = world[key];
			int localX = tx % chunkW;
			int localY = ty % chunkH;
			int index  = localY * chunkW + localX;
			chunk.c[index] = type;
			chunk.ogC[index] = type;
			dirtyKeys.insert(key);
		}
	}

	// Re-bake every chunk that changed
	for (int64_t key : dirtyKeys) {
		Chunk& chunk = world[key];
		if (chunk.tex) SDL_DestroyTexture(chunk.tex);
		chunk.tex = chunkTex(renderer, chunk, s, atlas);
	}
}
