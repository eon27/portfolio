#include <vector>

#ifndef TILE
#define TILE

#define EMPTY 0
#define GROUND 1
#define WALL 2
#define EDGE 3
#define EXIT 4
#define PLAYER 5

#define MAX_VISIBILITY 50.0f
#define VISIBILITY_RADIUS 5

#define ROW 0
#define COL 1

class Tile
{
private:
public:
	int visibility;
	int type;
	Tile(int type);
	~Tile();
};
#endif