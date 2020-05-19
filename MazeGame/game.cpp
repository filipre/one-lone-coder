#define OLC_PGE_APPLICATION
#include "../olcPixelGameEngine.h"

#include "mazeGeneration.h"

class MazeGame : public olc::PixelGameEngine
{
public:
	MazeGame()
	{
		sAppName = "Maze Game";
	}

public:
	bool OnUserCreate() override
	{
		int *maze = generateMaze(20, 20);
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		return true;
	}
};


int main()
{
	MazeGame game;
	if (game.Construct(50, 50, 9, 9))
		game.Start();

	return 0;
}
