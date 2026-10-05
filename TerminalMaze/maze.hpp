#include "tile.hpp"
#include "mazegen.hpp"

#ifndef MAZE
#define MAZE

class Maze
{
private:
	void generate_maze();
	void add_goal();
	mazegen::Generator gen;
public:
	std::vector<std::vector<Tile>> map;
	std::vector<int> pos; // [ROW, COLUMN]
	std::vector<int> goal;
	int size;
	Maze(int size);
	~Maze();

	void move_character(int input);
};

#endif