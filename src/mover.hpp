#include "helper.hpp"
#include "types.hpp"
#include <random>
#include "pathFinding.hpp"

using namespace Colors;

static std::uniform_int_distribution<int> RandomPos(1, 50);

class Mover{
	public:
		Pos pos;
		std::vector<Pos> path;
		Mover(Pos pos, std::unordered_map<int64_t, Chunk>& world) : pos(pos){
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
		
};
