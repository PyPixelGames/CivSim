#include "helper.hpp"
#include <random>
#include "pathFinding.hpp"

class Civ;

using namespace TextColor;
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
	std::pair<std::string, float> lastMutation = {"/", 0.0};

	mutable float cachedFitness = 0.0f;
	mutable bool dirty = true;

	Gene* find(const std::string& name) {
		auto it = genes.find(name);
		return it != genes.end() ? &it->second : nullptr;
	}

	float fitness() const {
		if (!dirty) return cachedFitness;

		float diff = 0;
		for (const auto& [name, gene] : genes) {
			diff += std::abs(gene.desiredValue - gene.value);
		}

		cachedFitness = 1.0f / (1.0f + diff);
		dirty = false;
		return cachedFitness;
	}

	void add(Gene gene) {
		genes.insert({gene.name, gene});
		dirty = true; // Mark for recalculation
	}

	bool operator==(const DNA& other) const {
		if (genes.size() != other.genes.size()) return false;
		for (const auto& [name, gene] : other.genes) {
			if (genes.find(name) == genes.end()) return false;
		}
		return true;
	}

	DNA crossover(const DNA& other) const {
		DNA result;
		// Optimization: checking size first is faster than full comparison
		if (genes.size() != other.genes.size()) {
			throw std::invalid_argument("Genes are not the same");
		}

		for (const auto& [name, gene] : genes) {
			int coinflip = std::uniform_int_distribution<int>(0, 1)(rng);
			result.add(coinflip == 0 ? gene : other.genes.at(name));
		}
		return result;
	}

	void mutate() {
		std::vector<std::string> keys;
		keys.reserve(genes.size());
		for (const auto& [k, v] : genes) keys.push_back(k);

		int index = std::uniform_int_distribution<size_t>(0, keys.size() - 1)(rng);
		const std::string& targetKey = keys[index];

		float mutation = std::uniform_real_distribution<float>(-0.1, 0.1)(rng);
		genes[targetKey].value += mutation;

		lastMutation = {targetKey, mutation};
		dirty = true;
	}

	void debug(){
		std::cout << green << "Fitness: " << blue << fitness() << reset << std::endl;
		if (lastMutation.first != "/"){
			std::cout << green << "Last mutation: " << darkRed << lastMutation.first << reset
				<< " was added " << blue << lastMutation.second << reset << std::endl;
		}
		std::cout << std::endl;
		for (auto& [name, gene] : genes){
			std::cout << darkRed << name << ": " << blue << gene << reset << std::endl;
		}
	}
};

class Creature {
	public:
		DNA dna;
		int id=0;
		int age=0;
		int maxAge=100;
		bool alive=true;

		AtlasPos img{};

		Civ* civ;

		Pos pos{};
		Pos goal{};

		Cell standingOn;
		std::unordered_map<std::string, float> state;

		std::vector<Pos> path;
		Mood mood;
		Meals meal;

		std::vector<Creature*> parents={};

		Creature(Pos pos,std::unordered_map<int64_t,Chunk>& /*world*/,
				int id, Civ* civ):id(id), pos(pos), civ(civ){
		}

		virtual Creature* spawn(Pos pos, std::unordered_map<int64_t, Chunk>& world, int id) const = 0;
		virtual void init(std::unordered_map<int64_t, Chunk>& /*world*/){};

		virtual void updateMood(std::unordered_map<int64_t, Chunk>& world){}

		void update(std::unordered_map<int64_t, Chunk>& world,
				std::vector <Creature*> creatures);

		void updatePosition(std::unordered_map<int64_t, Chunk>& world,Pos newPos);

		void pathFind(std::unordered_map<int64_t, Chunk>& world,Pos targetPos={-1, -1});

		Pos lookFor(std::unordered_map<int64_t, Chunk>& world,Cell target);

		void debug(){
			std::cout << cyan << "--- ID: " << id << " ---" << reset << std::endl;
			if (parents.size()){
				std::cout << magenta << "Parent ID's: " << parents[0]->id
					<< " | "<<parents[1]->id << reset << std::endl;
				std::cout << std::endl;
			}
			dna.debug();
			std::cout << cyan << "--------" << reset << std::endl;
			std::cout << std::endl;
		}

		float checkState(std::string state){
			auto it=this->state.find(state);
			if (it!=this->state.end()){
				return it->second;
			}else{
				return 0.0f;
			}
		}

		bool changeState(std::string state, float value){
			auto it=this->state.find(state);
			if (it!=this->state.end()){
				it->second = value;
				return true;
			}
			return false;
		}

		virtual ~Creature() = default;
};

template<typename Derived>
class CreatureBase : public Creature {
	public:
		CreatureBase(Pos pos, std::unordered_map<int64_t, Chunk>& world, int id, Civ* civ)
			: Creature(pos, world, id, civ) {}

		Creature* spawn(Pos pos, std::unordered_map<int64_t, Chunk>& world, int id) const override {
			return new Derived(pos, world, id, civ);  // pass civ down
		}
};


class Human : public CreatureBase<Human>{
	public:
		Human(Pos pos,std::unordered_map<int64_t,Chunk>& world,int id, Civ* civ)
			:CreatureBase(pos, world, id, civ){
				dna.add({"foodLove", std::uniform_real_distribution<float>(0.01f, 0.5f)(rng), 0.01});
				dna.add({"sight", std::uniform_real_distribution<float>(100.0f, 150.0f)(rng), 600.0f});
				dna.add({"agility", std::uniform_real_distribution<float>(1.0f, 3.0f)(rng), 5.0f});
				dna.add({"iq", std::uniform_real_distribution<float>(30.0f, 50.0f)(rng), 100.0f});
			}

		void init(std::unordered_map<int64_t, Chunk>& world) override {
			img.row=2;
			img.column=std::uniform_int_distribution<int>(0, 5)(rng);
			img.state=true;

			Cell c = checkCell(world, pos);
			c.entity = img;

			changeCell(world, pos, c, false);
			//pathFind(world);

			state.insert({"mealInSight", 1.0f});
		}

		void updateMood(std::unordered_map<int64_t, Chunk>& world) override;
};
