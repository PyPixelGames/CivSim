#include "levelGeneration.hpp"
#include "helper.hpp"
#include "FastNoiseLite.h"
#include "types.hpp"
#include <random>
#include <unordered_set>
#include <iostream>
#include <algorithm>

static int levelSizeX = 640;
static int levelSizeY = 640;
static std::mt19937 rng(std::random_device{}());
static std::uniform_real_distribution<float> dist(1.0f, 100000.0f);

static std::uniform_real_distribution<float> biomeBlend(0.0f, 1.0f);

int minRiverSize=50;
int minRiverAmount=8;
int maxRiverAttempts=50000;
static std::uniform_int_distribution<int> RiverWidth(3, 4);
static std::uniform_int_distribution<int> rp(0, levelSizeX - 1);


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
	float biomeVarX = dist(rng);
	float biomeVarY = dist(rng);

	std::unordered_set<int64_t> dirtyKeys;
	float heightmap[levelSizeX*levelSizeY];


	// Terrain generation
	for (int ty = 0; ty < levelSizeY; ty++) {
		for (int tx = 0; tx < levelSizeX; tx++) {
			ChunkCoord coords = toChunk(tx, ty);
			int64_t key = getKey(coords.cx, coords.cy);
			if (world.find(key) == world.end()) continue;

			Cell type{-1, -1};


			// Biome generation	
			float biomeValue = biomeNoise.GetNoise((float)(tx + biomeVarX),(float)(ty + biomeVarY));
			biomeValue = (biomeValue + 1.0f) / 2.0f; 

			if (biomeValue >= 0.605f){
				type.bg.x = SNOWY;
			}else if(biomeValue >= 0.595){
				float r = biomeBlend(rng);

				if (r>=0.5) {type.bg.x = FIELDS;}
				else        {type.bg.x = SNOWY;}
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


			// Actually applying the change
			Chunk& chunk  = world[key];
			chunk.c[coords.idx] = type;
			chunk.ogC[coords.idx] = type;
			dirtyKeys.insert(key);

			heightmap[ty * levelSizeX + tx] = terValue; // world position
		}
	}

	//Generate rivers
	int riverCount=0;
	int attempts=0;
	while (riverCount<minRiverAmount){
		if (attempts > maxRiverAttempts) break;
		Pos pos{rp(rng), rp(rng)};

		// Check the height of the pos to see if it is not fully flat or even worse, in water
		float startHeight = heightmap[pos.y * levelSizeX + pos.x];
		if (startHeight < 0.5f) continue;

		int riverWidth=RiverWidth(rng);
		bool cont=true;
		std::vector<Pos> river;

		// Trace the path of the river
		while (cont){
			std::pair<float, Pos> lastNeighbour;
			lastNeighbour.first = heightmap[pos.y*levelSizeX+pos.x];
			lastNeighbour.second = pos;
			for (int y=-1; y<=1; y++){
				for (int x=-1; x<=1; x++){
					if (x==0 && y==0) continue;

					Pos candidate{pos.x+x, pos.y+y};
					if (std::find(river.begin(), river.end(), candidate) != river.end()) continue;

					if (candidate.x < 0 || candidate.x >= levelSizeX ||
							candidate.y < 0 || candidate.y >= levelSizeY) continue;

					float height = heightmap[(pos.y+y)*levelSizeX+(pos.x+x)];
					if (height < lastNeighbour.first){
						lastNeighbour.first = height;
						lastNeighbour.second = candidate;
					}
				}
			}

			if (lastNeighbour.second == pos) break; //stuck

			ChunkCoord cCoords = toChunk(lastNeighbour.second.x, lastNeighbour.second.y);
			int64_t key = getKey(cCoords.cx, cCoords.cy);
			if (world.find(key) == world.end()) break;
			Chunk& chunk = world[key];

			if (chunk.c[cCoords.idx].bg.y == Water){
				cont=false;
			}

			pos=lastNeighbour.second;
			river.push_back(pos);
		}

		if (std::size(river) > minRiverSize){
			int length = std::size(river);
			// Carve water
			std::vector<Pos> carvedAdditions;
			for (int i = 0; i < river.size(); i++) {
				auto p = river[i];

				float t = (float)i / (float)(length - 1);

				// Taper from full riverWidth down to 1 at the mouth
				int currentWidth = std::max(1, (int)std::round(riverWidth * t));	

				for (int dy = -currentWidth; dy <= currentWidth; dy++) {
					for (int dx = -currentWidth; dx <= currentWidth; dx++) {
						if (dx*dx + dy*dy > currentWidth*currentWidth) continue;
						Pos wp{p.x + dx, p.y + dy};
						if (wp.x>0 && wp.x<levelSizeX && wp.y>0 && wp.y<levelSizeY){
							ChunkCoord cCoords = toChunk(wp.x, wp.y);
							int64_t key = getKey(cCoords.cx, cCoords.cy);
							if (world.find(key) == world.end()) continue;
							Chunk& chunk = world[key];
							dirtyKeys.insert(key);
							chunk.c[cCoords.idx].bg.y = Water;
							chunk.c[cCoords.idx].fg.state = false;

							carvedAdditions.push_back(wp);
						}
					}
				}
			}

			for (auto add: carvedAdditions){
				river.push_back(add);
			}

			// Sand edges — inside the if block, after water is carved
			for (auto cellPos : river){
				for (int y=-1; y<=1; y++){
					for (int x=-1; x<=1; x++){
						if (x==0 && y==0) continue; //skip center
						if (x*x + y*y > 1) continue; //skip corners

						Pos p{cellPos.x+x, cellPos.y+y};

						if (p.x<=0 || p.x>=levelSizeX || p.y<=0 || p.y>=levelSizeY) continue; //safety

						ChunkCoord c = toChunk(p.x, p.y);
						int64_t key = getKey(c.cx, c.cy);
						if (world.find(key) == world.end()) continue;
						Chunk& chunk = world[key];
						dirtyKeys.insert(key);

						int value = chunk.c[c.idx].bg.y;
						if (value != Water && value != Mountain && value != MountainTop){
							chunk.c[c.idx].bg.y = Sand;
							chunk.c[c.idx].fg.state = false;
						}
					}
				}
			}

			riverCount++;
			attempts=0;
			std::cout << riverCount << std::endl;
		} else {
			attempts++;
			river.clear();
		}
	}
	// Re-bake every chunk that changed
	for (int64_t key : dirtyKeys) {
		Chunk& chunk = world[key];
		if (chunk.tex) SDL_DestroyTexture(chunk.tex);
		chunk.tex = chunkTex(renderer, chunk, s, atlas);
	}
}
