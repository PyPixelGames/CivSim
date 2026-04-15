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

class Creature{
	public:
		Pos pos;
		std::vector<Pos> path;
		Mood mood;
		Meals meal;

		Creature(Pos pos,std::unordered_map<int64_t,Chunk>& world):pos(pos){
			bool search = true;
			Pos goal;
			while (search){
				goal.x = RandomPos(rng);
				goal.y = RandomPos(rng);
				Cell c = checkCell(world, goal);
				if (c.bg.y != Water && c.fg.state!=true){
					break;
				}
			}
			std::cout << goal.x << " - " << goal.y << std::endl;
			path = astar(pos, goal, world);
		}

		void update(std::unordered_map<int64_t, Chunk>& world);

		void updateMood();

	private:
		float FoodLove=0.1;	// How much the food effects the mood
		bool alive=true;
};
