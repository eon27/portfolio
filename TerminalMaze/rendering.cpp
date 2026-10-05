#include "rendering.hpp"

#define RESET_FORMAT "\x1b[48;5;0m"

void render(Maze map) {
	std::vector<std::vector<std::string>> render_buffer;
	clear_screen();
	for (size_t row = 0; row < map.size; row++)
	{
		for (size_t col = 0; col < map.size; col++)
		{
			if (map.pos[ROW] == row && map.pos[COL] == col) {
				std::cout << tile(PLAYER);
				std::cout << tile(PLAYER);
			} else {
				std::cout << tile(map.map[row][col]);
				std::cout << tile(map.map[row][col]);
			}
		}
		std::cerr << RESET_FORMAT << std::endl;
	}
}



std::string tile(Tile t) {
	std::string tilecode = "\x1b[48;2;";
	std::string start = "0;";
	std::string end = ";0";
	std::string intensity_g;
	std::string intensity_rb;
	switch (t.type)
	{
	case EMPTY:
		tilecode += "0";
		break;
	case GROUND:
		tilecode += start + std::to_string(25 + int(30.0f*(t.visibility/MAX_VISIBILITY))) + end;
		break;
	case WALL:
		tilecode += "0;25;0";
		break;
	case EDGE:
		tilecode += "0;10;0";
		break;
	case EXIT:
		intensity_g = std::to_string(25 + int(175.0f*(t.visibility/MAX_VISIBILITY)));
		intensity_rb = std::to_string(int(200.0f*(t.visibility/MAX_VISIBILITY)));
		tilecode += intensity_rb + ';' + intensity_g + ';' + intensity_rb;
		break;
	case PLAYER:
		tilecode += "0;220;0";
		break;
	
	default:
		tilecode += "255";
		break;
	}
	tilecode += "m ";
	return tilecode;
}

void hide_cursor() {
	printf("\x1b[?25l");
}

void show_cursor() {
	printf("\x1b[?25h");
}

void move_cursor(int row, int col) {
	printf("\x1b[%d,%dH", col, row);
}

void clear_screen() {
	printf("\x1b[H");
	printf("\x1b[2J");
}