#include <iostream>
#include <cstdlib>

#include "maze.hpp"

Maze::Maze(int size)
{
	this->size = size;
	pos.push_back(1);
	pos.push_back(1);

	generate_maze();
	add_goal();
}

void Maze::generate_maze() {
	mazegen::Config cfg;
	cfg.ROOM_BASE_NUMBER = 25;
	cfg.ROOM_SIZE_MIN = 4;
	cfg.ROOM_SIZE_MAX = 7;
	cfg.EXTRA_CONNECTION_CHANCE = 0.0;
	cfg.WIGGLE_CHANCE = 0.5;
	cfg.DEADEND_CHANCE = 0.5;
	cfg.RECONNECT_DEADENDS_CHANCE = 0.5;
	cfg.CONSTRAIN_HALL_ONLY = true;

	mazegen::PointSet constraints {{1, 1}, {size - 2, size - 2}};

	gen = mazegen::Generator();
	gen.generate(size, size, cfg, constraints);
	size = gen.maze_height();

	// Transform information from mazegen to this maze
	for (size_t i = 0; i < gen.maze_height(); i++) {
		std::vector<Tile> row;
		for (size_t j = 0; j < gen.maze_width(); j++) {
			if (gen.region_at(j, i) == mazegen::NOTHING_ID) {
				row.push_back(Tile(WALL));
			} else {
				row.push_back(Tile(GROUND));
			}
		}
		map.push_back(row);
	}
}

void Maze::add_goal() {
	int random_room = rand()%gen.get_rooms().size();
	int goal_x = (gen.get_rooms()[random_room].max_point.x + gen.get_rooms()[random_room].min_point.x) / 2;
	int goal_y = (gen.get_rooms()[random_room].max_point.y + gen.get_rooms()[random_room].min_point.y) / 2;

	map[goal_y][goal_x].type = EXIT;

	goal.push_back(goal_y);
	goal.push_back(goal_x);
}

Maze::~Maze()
{
}

void Maze::move_character(int input){
	switch (input) {
	case 'w':
		if (map[pos[ROW] - 1][pos[COL]].type != WALL) {
			pos[ROW] -= 1;
		}
		break;
	case 'a':
		if (map[pos[ROW]][pos[COL] - 1].type != WALL) {
			pos[COL] -= 1;
		}
		break;
	case 's':
		if (map[pos[ROW] + 1][pos[COL]].type != WALL) {
			pos[ROW] += 1;
		}
		break;
	case 'd':
		if (map[pos[ROW]][pos[COL] + 1].type != WALL) {
			pos[COL] += 1;
		}
		break;
	
	default:
		break;
	}

	// Decrease visibility of all tiles then max the visibility of tiles within circular radius VISIBLITY_RADIUS
	for (size_t i = 0; i < size; i++) {
		for (size_t j = 0; j < size; j++) {
			if (map[i][j].visibility > 0) map[i][j].visibility -= 1;
			if ( sqrt( ((pos[ROW] - i) * (pos[ROW] - i)) + ((pos[COL] - j) * (pos[COL] - j)) ) < VISIBILITY_RADIUS ) map[i][j].visibility = MAX_VISIBILITY;
		}
	}
}
