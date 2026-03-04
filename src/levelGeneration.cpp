#include "levelGeneration.hpp"
#include <random>
#include "FastNoiseLite.h"

FastNoiseLite noise;
int levelSizeX = 1000; int levelSizeY=1000;

std::vector<std::string> generateLevel(){
	static std::mt19937 rng(std::random_device{}());
	static std::uniform_real_distribution<float> dist(1.0f, 100000.0f);
	float varX = dist(rng);
	float varY = dist(rng);

	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

	std::vector<std::string> level;

	for (size_t y=0; y<levelSizeY; y++){
		std::string s = "";
		for(size_t x=0; x<levelSizeX; x++){
			float value = noise.GetNoise((x+varX), (y+varY));

			char shade; // Use a char instead of a string
			if (value >= 0.9f)      shade = '@';
			else if (value >= 0.8f) shade = '#';
			else if (value >= 0.6f) shade = '=';
			else if (value >= 0.4f) shade = '-';
			else if (value >= 0.2f) shade = '.';
			else                    shade = ' ';

			s += shade; // std::string handles += char perfectly

		}
		level.push_back(s);
	}

	return level;
}
