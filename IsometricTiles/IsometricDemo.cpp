#define OLC_PGE_APPLICATION
#include "../olcPixelGameEngine.h"

class IsometricDemo : public olc::PixelGameEngine
{
public:
    IsometricDemo()
    {
        sAppName = "Isometric Tiles";
    }

private:
    olc::vi2d vWorldSize = { 14, 10 };
    olc::vi2d vTileSize = { 40, 20 };
    olc::vi2d vOrigin = { 5, 1 };
    olc::Sprite *sprIsom = nullptr;
    int *pWorld = nullptr;

public:
    bool OnUserCreate() override
    {
        sprIsom = new olc::Sprite("IsometricTiles/isometric_demo.png");
        pWorld = new int[vWorldSize.x * vWorldSize.y]{ 0 };
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Clear(olc::WHITE);

        // mouse
        olc::vi2d vMouse = { GetMouseX(), GetMouseY() };
        olc::vi2d vCell = { vMouse.x / vTileSize.x, vMouse.y / vTileSize.y };
        olc::vi2d vOffset = { vMouse.x % vTileSize.x, vMouse.y % vTileSize.y };
        
        // sample color to get corner locations (cheat)
        olc::Pixel col = sprIsom->GetPixel(3 * vTileSize.x + vOffset.x, vOffset.y );

        // work out selected cell (screen to world)
        olc::vi2d vSelected =
        {
            (vCell.y - vOrigin.y) + (vCell.x - vOrigin.x), 
            (vCell.y - vOrigin.y) - (vCell.x - vOrigin.x)
        };
        olc::Pixel red = olc::RED;
		if (col.n == olc::RED.n) vSelected += {-1, +0};
		if (col.n == olc::BLUE.n) vSelected += {+0, -1};
		if (col.n == olc::GREEN.n) vSelected += {+0, +1};
		if (col.n == olc::YELLOW.n) vSelected += {+1, +0};

        if (GetMouse(0).bPressed)
        {
            std::cout << "clicked" << std::endl;
            if (vSelected.x >= 0 && vSelected.x < vWorldSize.x && vSelected.y >= 0 && vSelected.y < vWorldSize.y)
            {
                ++pWorld[vSelected.y * vWorldSize.x + vSelected.x] %= 6;
                std::cout << "new value " << pWorld[vSelected.y * vWorldSize.x + vSelected.x] << std::endl;
            }
        }
        
        auto ToScreen = [&](int x, int y)
        {
            return olc::vi2d
            {
                (vOrigin.x * vTileSize.x) + (x - y) * (vTileSize.x / 2),
                (vOrigin.y * vTileSize.y) + (x + y) * (vTileSize.y / 2)
            };
        };

        SetPixelMode(olc::Pixel::MASK);

        for (int y = 0; y < vWorldSize.y; y++)
        {
            for (int x = 0; x < vWorldSize.x; x++)
            {
                olc::vi2d vWorld = ToScreen(x, y);
                switch(pWorld[y*vWorldSize.x + x])
                {
                case 0:
					// Invisble Tile
					DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, 1 * vTileSize.x, 0, vTileSize.x, vTileSize.y);
					break;
				case 1:
					// Visible Tile
					DrawPartialSprite(vWorld.x, vWorld.y, sprIsom, 2 * vTileSize.x, 0, vTileSize.x, vTileSize.y);
					break;
				case 2:
					// Tree
					DrawPartialSprite(vWorld.x, vWorld.y - vTileSize.y, sprIsom, 0 * vTileSize.x, 1 * vTileSize.y, vTileSize.x, vTileSize.y * 2);
					break;
				case 3:
					// Spooky Tree
					DrawPartialSprite(vWorld.x, vWorld.y - vTileSize.y, sprIsom, 1 * vTileSize.x, 1 * vTileSize.y, vTileSize.x, vTileSize.y * 2);
					break;
				case 4:
					// Beach
					DrawPartialSprite(vWorld.x, vWorld.y - vTileSize.y, sprIsom, 2 * vTileSize.x, 1 * vTileSize.y, vTileSize.x, vTileSize.y * 2);
					break;
				case 5:
					// Water
					DrawPartialSprite(vWorld.x, vWorld.y - vTileSize.y, sprIsom, 3 * vTileSize.x, 1 * vTileSize.y, vTileSize.x, vTileSize.y * 2);
					break;
                }
            }
        }

        // draw selected cell
        SetPixelMode(olc::Pixel::ALPHA);
        olc::vi2d vSelectedWorld = ToScreen(vSelected.x, vSelected.y);
        DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, sprIsom, 0 * vTileSize.x, 0, vTileSize.x, vTileSize.y);
        SetPixelMode(olc::Pixel::NORMAL);

        // SetPixelMode(olc::Pixel::ALPHA);

        return true;
    }

};

int main()
{
    IsometricDemo demo;
    if (demo.Construct(512, 300, 2, 2))
        demo.Start();
    return 0;
}