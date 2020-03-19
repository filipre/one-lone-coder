#define OLC_PGE_APPLICATION

#include <iostream>
#include "olcPixelGameEngine.h"

class Tetris : public olc::PixelGameEngine {
    private:
        std::wstring tetromino[8];
        unsigned char *pField = nullptr;
        int nCurrentPiece;
        int nCurrentRotation;
        int nCurrentX;
        int nCurrentY;
        bool bGameOver = false;
        int nLines = 0;
        int nScore = 0;
        int nLevel = 0;
        double dKeyPressTime = 0;
        double dCountTime = 0;
        double dLineTime = 0;
        std::vector<int> vLines;

        olc::Pixel pBlocks[10] = { 
            olc::WHITE,     // blank
            olc::DARK_BLUE,     // tetromino 1
            olc::DARK_GREEN, 
            olc::VERY_DARK_GREEN, 
            olc::VERY_DARK_CYAN, 
            olc::VERY_DARK_BLUE, 
            olc::DARK_CYAN, 
            olc::CYAN,     // tetromino 7
            olc::YELLOW,      // line
            olc::BLACK      // border
        };

        double pSpeeds[4] = { 0.9, 0.5, 0.25, 0.1 };

        static int Rotate(int px, int py, int r)
        {
            int pi = 0;
            switch (r % 4)
            {
            case 0:
                pi = 0 + px + py*4;
                break;
            case 1:
                pi = 12 + py - px*4;
                break;
            case 2:
                pi = 15 - px - py*4;
                break;
            case 3:
                pi = 3 - py + px*4;
                break;
            }
            return pi;
        }

        bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
        {
            for (int px = 0; px < 4; px++)
            {
                for (int py = 0; py < 4; py++)
                {
                    int pi = Rotate(px, py, nRotation);
                    int fi = (nPosY + py)*ScreenWidth() + (nPosX + px);
                    if (nPosX + px >= 0 && nPosX + px < ScreenWidth())
                    {
                        if (nPosY + py >= 0 && nPosY + py < ScreenHeight())
                        {
                            if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
                                return false;
                        }
                    }
                }
            }
            return true;
        }

    public:
        Tetris() {
            sAppName = "Tetris";
        }

    public:
        bool OnUserCreate() override 
        {
            // Called once at the start, so create things here
            tetromino[0].append(L"................"); // blank, makes things easier
            tetromino[1].append(L"..X...X...X...X.");
            tetromino[2].append(L"..X..XX...X.....");
            tetromino[3].append(L".....XX..XX.....");
            tetromino[4].append(L"..X..XX..X......");
            tetromino[5].append(L".X...XX...X.....");
            tetromino[6].append(L".X...X...XX.....");
            tetromino[7].append(L"..X...X..XX.....");

            nCurrentPiece = 3;
            nCurrentRotation = 0;
            nCurrentX = ScreenWidth()/2 - 2;
            nCurrentY = 0;

            pField = new unsigned char[ScreenWidth() * ScreenHeight()];
            for (int x = 0; x < ScreenWidth(); x++)
                for (int y = 0; y < ScreenHeight(); y++)
                    pField[y*ScreenWidth() + x] = (x == 0 || x == ScreenWidth()-1 || y == ScreenHeight()-1) ? 9 : 0;

            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override 
        {
            if (bGameOver)
            {
                return true;
            }

            // Check if we have some line animation
            if (!vLines.empty())
            {
                dLineTime += fElapsedTime;
                if (dLineTime < 0.5)
                    return true;

                dLineTime = 0;
                for (auto &v : vLines)
                {
                    for (int px = 1; px < ScreenWidth() - 1; px++)
                    {
                        for (int py = v; py > 0; py--) 
                            pField[py*ScreenWidth() + px] = pField[(py-1)*ScreenWidth() + px];
                    }
                }
                vLines.clear();
            }

            // Input
            dKeyPressTime += fElapsedTime;
            if (dKeyPressTime >= 0.1 && GetKey(olc::Key::LEFT).bHeld)
            {
                dKeyPressTime = 0;
                nCurrentX += (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX-1, nCurrentY)) ? -1 : 0;
            }
            if (dKeyPressTime >= 0.1 && GetKey(olc::Key::RIGHT).bHeld)
            {
                dKeyPressTime = 0;
                nCurrentX += (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX+1, nCurrentY)) ? 1 : 0;
            }
            if (dKeyPressTime >= 0.05 && GetKey(olc::Key::DOWN).bHeld)
            {
                dKeyPressTime = 0;
                nCurrentY += (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY+1)) ? 1 : 0;
            }
            nCurrentRotation += (GetKey(olc::Key::Z).bPressed && DoesPieceFit(nCurrentPiece, nCurrentRotation+1, nCurrentX, nCurrentY)) ? 1 : 0;

            // Force Down
            dCountTime += fElapsedTime;
            if (dCountTime >= pSpeeds[(nLevel >= 4) ? 3 : nLevel])
            {
                dCountTime = 0;

                // Test if piece can go down
                if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY+1))
                {
                    nCurrentY += 1;
                }
                else
                {
                    // lock piece
                    for (int px = 0; px < 4; px++)
                        for (int py = 0; py < 4; py++)
                            if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                                pField[(nCurrentY+py)*ScreenWidth() + (nCurrentX+px)] = nCurrentPiece;

                    // check lines
                    for (int py = 0; py < 4; py++)
                    {
                        if (nCurrentY + py < ScreenHeight() - 1)
                        {
                            bool bLine = true;
                            for (int px = 1; px < ScreenWidth() - 1; px++)
                                bLine &= pField[(nCurrentY+py)*ScreenWidth() + px] != 0;
                            if (bLine)
                            {
                                for (int px = 1; px < ScreenWidth() - 1; px++)
                                    pField[(nCurrentY+py)*ScreenWidth() + px] = 8; // special "line" state
                                vLines.push_back(nCurrentY+py); // "remember line to delete it soon"
                            }
                        }
                    }

                    // update level, score, speed
                    nLevel += (nLines+vLines.size())/10 - nLines/10;
                    nLines += vLines.size();
                    nScore += 25;
                    nScore += (1 << vLines.size()) * 100;

                    // pick new piece
                    nCurrentX = ScreenWidth() / 2 - 2;
                    nCurrentY = 0;
                    nCurrentRotation = 0;
                    nCurrentPiece = rand() % 7 + 1; // [1, 7]

                    std::cout << nScore << " " << nLines << " " << nLevel << std::endl;

                    // game over if new piece does not fit straight in
                    bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
                }                
            }

            // Display
            Clear(olc::BLACK);

            // Draw Field
            for (int x = 0; x < ScreenWidth(); x++)
                for (int y = 0; y < ScreenHeight(); y++)
                    Draw(x, y, pBlocks[pField[y*ScreenWidth() + x]]);

            // Draw Current Piece
            for (int px = 0; px < 4; px++)
                for (int py = 0; py < 4; py++)
                    if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                        Draw(nCurrentX+px, nCurrentY+py, pBlocks[nCurrentPiece]);

            return true;
        }
};

int main()
{
    int nFieldWidth = 12;
    int nFieldHeight = 18;
    int nBlockWidth = 32;
    int nBlockHeight = 32;

    Tetris tetris;
	if (tetris.Construct(nFieldWidth, nFieldHeight, nBlockWidth, nBlockHeight))
		tetris.Start();

	return 0;
}
