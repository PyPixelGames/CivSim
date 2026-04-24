#include "helper.hpp"
#include <random>
#include "pathFinding.hpp"

using namespace Colors;

static std::uniform_int_distribution<int> RandomPos(1, 50);

struct Mood {
	short int val=1;
	float happiness=0; //-val(sad) -- val (happy)
	float chillness=0; //-val(angry) -- val(relaxed/calm)
	float entertained=0; //-val(bored) -- val(entertained)
	float workSatisfaction=0; //-val(doing work they hate) -- val(good)

	float evaluate(){
		return (happiness+chillness+entertained+workSatisfaction)/4;
	}

	void clamp(){
		happiness = clampFloat(happiness, -val, val);
		chillness = clampFloat(chillness, -val, val);
		entertained = clampFloat(entertained, -val, val);
		workSatisfaction = clampFloat(workSatisfaction, -val, val);
	}
};

struct Meals{
	short int val=1;
	float food = 0.5; //-1(starving) -- 1(full with nice meal)
	float drink = 0; //-1(dehydration) -- 1(full with nice fluid)

	float evaluate(){
		return (food+drink)/2;
	}

	void clamp(){
		food = clampFloat(food, -val, val);
		drink = clampFloat(drink, -val, val);
	}
};

struct Gene{
	std::string name="EMPTY";
	float value=0.0f;
	float desiredValue=0.0f;

	operator float() const {return value;};
};

struct DNA {
	std::unordered_map<std::string, Gene> genes;

	Gene* find(const std::string& name) {
		auto it = genes.find(name);
		return it != genes.end() ? &it->second : nullptr;
	}

	float fitness() const {
		float fitness = 0;
		for (auto& [name, gene] : genes) {
			fitness += std::abs(gene.desiredValue - gene.value);
		}
		return fitness;
	}

	void add(Gene gene){
		genes.insert({gene.name, gene});
	}
};


class Creature{
	public:
		Creature(Pos pos,std::unordered_map<int64_t,Chunk>& world,
				int id=0):pos(pos), id(id){
			Pos goal;
			bool search = true;
			while (search){
				goal.x = RandomPos(rng);
				goal.y = RandomPos(rng);
				Cell c = checkCell(world, goal);
				if (c.bg.column != Water && c.fg.state!=true){
					break;
				}
			}
			std::cout << goal.x << " - " << goal.y << std::endl;
			path = astar(pos, goal, world);


		}

		void update(std::unordered_map<int64_t, Chunk>& world,
				std::vector <Creature*> creatures);

		void updateMood();

	protected:
		bool alive=true;
		Pos pos;
		Cell cell{};

		std::vector<Pos> path;
		Mood mood;
		Meals meal;

		int id;

		DNA dna;
};

class Human : public Creature{
	public:
		Human(Pos pos,std::unordered_map<int64_t,Chunk>& world,int id)
			:Creature(pos, world, id){
				//constructor unique to this specific creature

				//Modify the Genes from the parent creature class so all the parent DNA
				//functions work when they will be implemented

				dna.add({"foodLove", std::uniform_real_distribution<float>(0.01f, 0.5f)(rng), 0.01});
				dna.add({"sight", std::uniform_real_distribution<float>(3.0f, 15.0f)(rng), 15.0f});

				cell.fg.row=2; cell.fg.column=id; cell.fg.state=true;
				cell.bg.row=0; cell.bg.column=0; cell.bg.state=true;
		}
};
