// #define OLC_PGE_APPLICATION
// #include "../olcPixelGameEngine.h"

#include <iostream>
#include <stack>

int *generateMaze(int nWidth, int nHeight) 
{
    enum 
    {
        CELL_PATH_N = 0x01,
        CELL_PATH_E = 0x02,
        CELL_PATH_S = 0x04,
        CELL_PATH_W = 0x08,
        CELL_VISITED = 0x10,
    };

    int *maze = new int[nWidth * nHeight];
    memset(maze, 0x00, nWidth * nHeight * sizeof(int));

    std::stack<std::pair<int, int>> stack; // (x, y) coordinates
    int x = rand() % nWidth;
    int y = rand() % nHeight;
    stack.push(std::make_pair(x, y));
    maze[y * nWidth + x] = CELL_VISITED;

    int nVisitedCells = 1;

    auto offset = [&](int x, int y)
    {
        return (stack.top().second + y) * nWidth + (stack.top().first + x);
    };

    while (nVisitedCells < nWidth*nHeight)
    {
        std::vector<int> neighbours; // unvisited neighbours
        if (stack.top().second > 0 && (maze[offset(0, -1)] & CELL_VISITED) == 0)
        {
            neighbours.push_back(0);
        } 
        if (stack.top().first < nWidth-1 && (maze[offset(1, 0)] & CELL_VISITED) == 0)
        {
            neighbours.push_back(1);
        }
        if (stack.top().second < nHeight-1 && (maze[offset(0, 1)] & CELL_VISITED) == 0)
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
            }

            nVisitedCells++;
            // std::cout << nVisitedCells << " / " <<  nMazeWidth*nMazeHeight << std::endl;
        }
        else
        {
            stack.pop();
        }
    }

    return maze;
}