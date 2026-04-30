#pragma once

#include <vector>
#include "creature.hpp"
#include "types.hpp"

class Civ{
	public:
		int id = 0;
		std::vector <Creature*> pending;
		std::vector <Creature*> creatures;
		Pos location;

		Civ(){};
		void update(std::unordered_map<int64_t, Chunk>& world);
		void clear();
		void evolve(std::unordered_map<int64_t, Chunk>& world);
};
