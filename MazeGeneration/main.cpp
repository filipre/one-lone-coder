#define OLC_PGE_APPLICATION
#include "../olcPixelGameEngine.h"

#include <iostream>
#include <stack>

using namespace std::literals;

class MazeGeneration : public olc::PixelGameEngine
{
public:
	MazeGeneration()
	{
		sAppName = "Maze Generation";
	}

private:
    int nMazeWidth;
    int nMazeHeight;
    int *maze; // array

    enum 
    {
        CELL_PATH_N = 0x01,
        CELL_PATH_E = 0x02,
        CELL_PATH_S = 0x04,
        CELL_PATH_W = 0x08,
        CELL_VISITED = 0x10,
    };

    int nVisitedCells;
    std::stack<std::pair<int, int>> stack; // (x, y) coordinates
    int nPathWidth;

public:
	bool OnUserCreate() override
	{
        nMazeWidth = 40;
        nMazeHeight = 25;
        maze = new int[nMazeWidth * nMazeHeight];
        memset(maze, 0x00, nMazeWidth * nMazeHeight * sizeof(int));
        nPathWidth = 3;

        // starting cell
        int x = rand() % nMazeWidth;
        int y = rand() % nMazeHeight;
        stack.push(std::make_pair(x, y));
        maze[y * nMazeWidth + x] = CELL_VISITED;
        nVisitedCells = 1;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        std::this_thread::sleep_for(10ms);

        auto offset = [&](int x, int y)
        {
            return (stack.top().second + y) * nMazeWidth + (stack.top().first + x);
        };

        // One iteration of the maze generation algorithm
        if (nVisitedCells < nMazeWidth*nMazeHeight)
        {
            std::vector<int> neighbours; // unvisited neighbours
            if (stack.top().second > 0 && (maze[offset(0, -1)] & CELL_VISITED) == 0)
            {
                neighbours.push_back(0);
            } 
            if (stack.top().first < nMazeWidth-1 && (maze[offset(1, 0)] & CELL_VISITED) == 0)
            {
                neighbours.push_back(1);
            }
            if (stack.top().second < nMazeHeight-1 && (maze[offset(0, 1)] & CELL_VISITED) == 0)
            {
                neighbours.push_back(2);
            }
            if (stack.top().first > 0 && (maze[offset(-1, 0)] & CELL_VISITED) == 0) 
                neighbours.push_back(3);

            if (!neighbours.empty())
            {
                int next_cell_direction = neighbours[rand() % neighbours.size()];

                switch (next_cell_direction)
                {
                case 0: // North
                    maze[offset(0, -1)] |= CELL_VISITED | CELL_PATH_S;
                    maze[offset(0, 0)] |= CELL_PATH_N;
                    stack.push(std::make_pair(stack.top().first + 0, stack.top().second - 1));
                    break;
                case 1: // East
                    maze[offset(1, 0)] |= CELL_VISITED | CELL_PATH_W;
                    maze[offset(0, 0)] |= CELL_PATH_E;
                    stack.push(std::make_pair(stack.top().first + 1, stack.top().second + 0));
                    break;
                case 2: // South
                    maze[offset(0, 1)] |= CELL_VISITED | CELL_PATH_N;
                    maze[offset(0, 0)] |= CELL_PATH_S;
                    stack.push(std::make_pair(stack.top().first + 0, stack.top().second + 1));
                    break;
                case 3: // West
                    maze[offset(-1, 0)] |= CELL_VISITED | CELL_PATH_E;
                    maze[offset(0, 0)] |= CELL_PATH_W;
                    stack.push(std::make_pair(stack.top().first - 1, stack.top().second + 0));
                    break;
                default:
                    std::cout << "should never happen" << std::endl;
                }

                nVisitedCells++;
                // std::cout << nVisitedCells << " / " <<  nMazeWidth*nMazeHeight << std::endl;
            }
            else
            {
                stack.pop();
            }

        }

        // Display
        Clear(olc::BLACK);

        // Draw Maze
        for (int x = 0; x < nMazeWidth; x++)
        {
            for (int y = 0; y < nMazeHeight; y++)
            {
                // individual cells
                for (int py = 0; py < nPathWidth; py++)
                {
                    for (int px = 0; px < nPathWidth; px++)
                    {
                        int draw_x = x*(nPathWidth+1) + px;
                        int draw_y = y*(nPathWidth+1) + py;
                        if (maze[y * nMazeWidth + x] & CELL_VISITED)
                            Draw(draw_x, draw_y, olc::WHITE);
                        else
                            Draw(draw_x, draw_y, olc::BLUE);
                    }
                }
                // passages
                for (int p = 0; p < nPathWidth; p++)
                {
                    if (maze[y * nMazeWidth + x] & CELL_PATH_S)
                        Draw(x*(nPathWidth+1) + p, y*(nPathWidth+1) + nPathWidth);
                    if (maze[y*nMazeWidth + x] & CELL_PATH_E)
                        Draw(x*(nPathWidth+1) + nPathWidth, y*(nPathWidth+1) + p);
                }
            }
        }

        // Draw top of stack
        for (int py = 0; py < nPathWidth; py++)
            for (int px = 0; px < nPathWidth; px++)
                Draw(stack.top().first*(nPathWidth+1) + px, stack.top().second*(nPathWidth+1) + py, olc::GREEN);

        return true;
	}
};


int main()
{
    // srand(clock());
	MazeGeneration app;
	if (app.Construct(160, 100, 5, 5))
		app.Start();

	return 0;
}
