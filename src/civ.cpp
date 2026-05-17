#include <iostream>
#include "civ.hpp"
#include "creature.hpp"

void Civ::update(std::unordered_map<int64_t, Chunk>& world){
	for (size_t i=0; i<creatures.size(); i++){
		Creature *c = creatures[i];
		c->update(world, creatures);
	}

	for (auto p: pending){
		p->init(world);
		creatures.push_back(p);
	}
	pending.clear();
}

void Civ::clear() {
    for (auto* c : pending) delete c;
    pending.clear();

    for (auto* c : creatures) delete c;
    creatures.clear();
}

void Civ::evolve(std::unordered_map<int64_t, Chunk>& world){
	if (creatures.size() < 2) {
		std::cout << "Not enough creatures to evolve (need at least 2)\n";
		return;
	}

	// Cache up fitness
	std::vector<std::pair<float, Creature*>> scored;
	scored.reserve(creatures.size());
	for (Creature* c : creatures)
		scored.push_back({c->dna.fitness(), c});

	//Sort by fitness with biggest being first
	std::sort(scored.begin(), scored.end(),[](const auto& a, const auto& b) {
		return a.first > b.first;
		});

	DNA newDNA = scored[0].second->dna.crossover(scored[1].second->dna);
	Creature* c = scored[0].second->spawn({0, 0}, world, this->id);

	newDNA.mutate();

	c->dna=newDNA;
	c->parents.push_back(scored[0].second);
	c->parents.push_back(scored[1].second);

	this->pending.push_back(c);
	this->id++;
}

void Civ::printStats(){
	std::cout << "\nAmount of creatures: " << creatures.size() << std::endl;
	auto it = std::max_element(creatures.begin(), creatures.end(),[](Creature* a, Creature* b) {
        return a->dna.fitness() < b->dna.fitness();
    });
	Creature* best = (it != creatures.end()) ? *it : nullptr;

	auto it2 = std::find_if(creatures.begin(), creatures.end(),[](Creature* c) {
        return c->id == 0;
    });
	Creature* first = (it2 != creatures.end()) ? *it2 : nullptr;

	std::cout << std::endl;
	std::cout << cyan << bold << "--- STATS ---" << reset << std::endl;
	std::cout << yellow << "ID 0 fitness: " << blue << first->dna.fitness() << reset << std::endl;
	std::cout << std::endl;

	std::cout << yellow << "Best: "<< reset << std::endl;
	std::cout << std::endl;
	best->debug();

	std::cout << green << "Improvement: " << blue <<
		best->dna.fitness()-first->dna.fitness() << reset << std::endl;
	std::cout << cyan << bold << "-------------" << reset << std::endl;
	std::cout << std::endl;
}
