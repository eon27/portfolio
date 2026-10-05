#include <string>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <termios.h>

#include "rendering.hpp"
#include "maze.hpp"

char getch();

int main(int argc, char *argv[]) {
	int size = 20;
	try {
		if (argc > 1) {
			size = std::stoi(argv[1]);
		}
	}
	catch(const std::invalid_argument& e) {
		std::cerr << "input argument must be a number" << '\n';
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
	}

	Maze map(size);

	hide_cursor();
	map.move_character(' ');
	render(map);

	if (argc > 2 && std::string(argv[2]) == "-idle") {
		std::vector<bool> visited(size * size, 0);
		std::vector<char> history;
		while (map.pos != map.goal)
		{
			// Check around player if tile is free (not wall) and that the player hasnt been there before
			std::vector<int> dirs;
			if (map.map[map.pos[ROW] - 1][map.pos[COL]].type != WALL && !visited[(map.pos[ROW] - 1) * size + map.pos[COL]]) dirs.push_back('w');
			if (map.map[map.pos[ROW]][map.pos[COL] - 1].type != WALL && !visited[(map.pos[ROW]) * size + map.pos[COL] - 1]) dirs.push_back('a');
			if (map.map[map.pos[ROW] + 1][map.pos[COL]].type != WALL && !visited[(map.pos[ROW] + 1) * size + map.pos[COL]]) dirs.push_back('s');
			if (map.map[map.pos[ROW]][map.pos[COL] + 1].type != WALL && !visited[(map.pos[ROW]) * size + map.pos[COL] + 1]) dirs.push_back('d');
			
			char dir;
			// If no direction is free move back
			if (dirs.empty()) {
				switch (history.back())
				{
				case 'w':
					dir = 's';
					break;
				case 'a':
					dir = 'd';
					break;
				case 's':
					dir = 'w';
					break;
				case 'd':
					dir = 'a';
					break;
				}
				history.pop_back();
			} else {
				// Move to random free direction
				dir = dirs[rand()%dirs.size()];
				history.push_back(dir);
			}
			map.move_character(dir);
			render(map);

			// Mark that current position is visited
			visited[(map.pos[ROW]) * size + map.pos[COL]] = 1;
			usleep(50000);
		}
		

	} else {

		char input;
		while (input = getch())
		{
			if (27 == input) {
				break;	
			}
			map.move_character(input);
			render(map);
			if (map.pos == map.goal) break;
		}
	}
	

	show_cursor();
	exit(0);	
}

char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}