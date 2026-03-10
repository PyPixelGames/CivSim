#include "helper.hpp"
#include "types.hpp"

using namespace Colors;

class Mover{
	public:
		int x;
		int y;

		Mover(int x, int y) : x(x), y(y){

		}

		void update(std::unordered_map<int64_t, Chunk>& world);

		void changeCell(std::unordered_map<int64_t, Chunk>& world, std::pair<int, int> pos,
				char c, SDL_Color color);
	
};
