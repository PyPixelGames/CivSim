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
		float diff = 0;
		for (auto& [name, gene] : genes) {
			diff += std::abs(gene.desiredValue - gene.value);
		}
		return 1.0f / (1.0f + diff);
	}

	void add(Gene gene){
		genes.insert({gene.name, gene});
	}

	bool operator==(const DNA& other) const{
		for (auto& [name, gene] : other.genes){
			if (genes.count(name) == 0){
				return false;
			}
		}
		return genes.size()==other.genes.size();
	}

	DNA crossover(const DNA& other) const{
		DNA result;

		if (!(*this==other)) {
			throw std::invalid_argument("Genes are not the same");
		}

		for (auto& [name, gene] : other.genes){
			int coinflip = std::uniform_int_distribution<int>(0, 1)(rng);
			std::cout << "Coinflip:  " << coinflip << std::endl;

			if (coinflip==0){
				result.add(genes.at(name));
			}else{
				result.add(other.genes.at(name));
			}
		}

		return result;
	}
};


class Creature {
	public:
		DNA dna;
		int id;
		int age=0;
		int maxAge=100;
		bool alive=true;
		Pos pos;
		Cell cell{};

		Pos goal;

		std::vector<Pos> path;
		Mood mood;
		Meals meal;

		Creature(Pos pos,std::unordered_map<int64_t,Chunk>& world,
				int id=0):pos(pos), id(id){
		}

		virtual Creature* spawn(Pos pos, std::unordered_map<int64_t, Chunk>& world, int id) const = 0;
		void update(std::unordered_map<int64_t, Chunk>& world,
				std::vector <Creature*> creatures);

		void updateMood();
		virtual ~Creature() = default;
};

template<typename Derived>
class CreatureBase : public Creature {
public:
    CreatureBase(Pos pos, std::unordered_map<int64_t, Chunk>& world, int id)
        : Creature(pos, world, id) {}

    Creature* spawn(Pos pos, std::unordered_map<int64_t, Chunk>& world, int id) const override {
        return new Derived(pos, world, id);
    }
};


class Human : public CreatureBase<Human>{
	public:
		Human(Pos pos,std::unordered_map<int64_t,Chunk>& world,int id)
			:CreatureBase(pos, world, id){
				dna.add({"foodLove", std::uniform_real_distribution<float>(0.01f, 0.5f)(rng), 0.01});
				dna.add({"sight", std::uniform_real_distribution<float>(3.0f, 15.0f)(rng), 15.0f});

				cell.fg.row=2; cell.fg.column=id; cell.fg.state=true;
				cell.bg.row=0; cell.bg.column=0; cell.bg.state=true;
				changeCell(world, pos, cell, false);

				//bool search = false;
				//while (search){
				//goal.x = RandomPos(rng);
				//goal.y = RandomPos(rng);
				//Cell c = checkCell(world, goal);
				//if (c.bg.column != Water && c.fg.state!=true){
				//break;
				//}
				//}
				//std::cout << goal.x << " - " << goal.y << std::endl;
				//path = astar(pos, goal, world);
			}
};
