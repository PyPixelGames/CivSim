#include <iostream>
#include "civ.hpp"

void Civ::update(std::unordered_map<int64_t, Chunk>& world){
	for (size_t i=0; i<creatures.size(); i++){
		Creature *c = creatures[i];
		c->update(world, creatures);
	}
}

void Civ::clear(){
	for (auto ptr : creatures){
		delete ptr;
	}
	creatures.clear();
}
