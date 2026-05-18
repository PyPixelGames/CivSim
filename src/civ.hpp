#pragma once

#include <vector>
#include "types.hpp"
#include "helper.hpp"

class Creature;

class Civ{
	public:
		int id = 0;
		std::vector <Creature*> pending;
		std::vector <Creature*> creatures;
		Pos location;

		std::unordered_set<int64_t> claimedCells;

		Civ(){};
		~Civ();
		void update(std::unordered_map<int64_t, Chunk>& world);
		void evolve(std::unordered_map<int64_t, Chunk>& world);
		void printStats();
};
