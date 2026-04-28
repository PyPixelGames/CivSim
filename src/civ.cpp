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

void Civ::evolve(std::unordered_map<int64_t, Chunk>& world){
	// Cache up fitness
	std::vector<std::pair<float, Creature*>> scored;
	scored.reserve(creatures.size());
	for (Creature* c : creatures)
		scored.push_back({c->dna.fitness(), c});

	//Sort by fitness with biggest being first
	std::sort(scored.begin(), scored.end(),
			[](const auto& a, const auto& b) {
			return a.first > b.first;
			});

	std::cout << "parents ID's  " << scored[0].second->id <<"|"<<scored[1].second->id <<std::endl;

	DNA newDNA = scored[0].second->dna.crossover(scored[1].second->dna);
	Creature* c = scored[0].second->spawn({1, 0}, world, this->id);

	c->dna=newDNA;
	this->creatures.push_back(c);
	this->id++;

	std::cout << "Id: " << c->id << "   Fitness: "
		<< c->dna.fitness() << std::endl;

}
