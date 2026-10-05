#include <vector>
#include <string>
#include <iostream>

#include "maze.hpp"

#ifndef RENDERING
#define RENDERING

void render(Maze map);

std::string tile(Tile t);

void hide_cursor();

void show_cursor();

void move_cursor(int row, int col);

void clear_screen();

#endif