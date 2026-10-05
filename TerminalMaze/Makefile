CXX = g++
WFLAGS  = -Wall -std=c++11 -Wformat -Wno-unknown-pragmas
	
mazegame: game.o maze.o rendering.o
	$(CXX) game.o maze.o rendering.o tile.o -o mazegame

game.o: rendering.o maze.o game.cpp
	$(CXX) -c game.cpp

rendering.o: rendering.cpp maze.o
	$(CXX) -c rendering.cpp

maze.o: maze.cpp tile.o
	$(CXX) -c maze.cpp

tile.o: tile.cpp
	$(CXX) -c tile.cpp

clean:
	rm -f game.o rendering.o maze.o tile.o mazegame
