#define OLC_PGE_APPLICATION
#include "../olcPixelGameEngine.h"

class PanAndZoom : public olc::PixelGameEngine
{
public:
    PanAndZoom()
    {
        sAppName = "Panning and Zooming";
    }

private:
    float fOffsetX = 0.0f;
    float fOffsetY = 0.0f;

    float fStartPanX = 0.0f;
    float fStartPanY = 0.0f;

    float fScaleX = 1.0f;
    float fScaleY = 1.0f;

    float fSelectedCellX = 0.0f;
    float fSelectedCellY = 0.0f;

    void WorldToScreen(float fWorldX, float fWorldY, int &nScreenX, int &nScreenY)
    {
        nScreenX = (int)((fWorldX - fOffsetX) * fScaleX);
        nScreenY = (int)((fWorldY - fOffsetY) * fScaleY);
    }

    void ScreenToWorld(int nScreenX, int nScreenY, float &fWorldX, float &fWorldY)
    {
        fWorldX = (float)nScreenX / fScaleX + fOffsetX;
        fWorldY = (float)nScreenY / fScaleY + fOffsetY;
    }

public:
    bool OnUserCreate() override
    {
        fOffsetX = -ScreenWidth() / 2;
        fOffsetY = -ScreenHeight() / 2;
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override 
    {
        // Input
        float fMouseX = (float)GetMouseX();
        float fMouseY = (float)GetMouseY();

        if (GetMouse(0).bPressed)
        {
            fStartPanX = fMouseX;
            fStartPanY = fMouseY;
        }

        if (GetMouse(0).bHeld)
        {
            fOffsetX -= (fMouseX - fStartPanX) / fScaleX;
            fOffsetY -= (fMouseY - fStartPanY) / fScaleY;
            // "new" pan for next epoch
            fStartPanX = fMouseX;
            fStartPanY = fMouseY;
        }

        // capture location of cursor before and after scale change
        float fMouseWorldX_before, fMouseWorldY_before;
        ScreenToWorld(fMouseX, fMouseY, fMouseWorldX_before, fMouseWorldY_before);
        
        if (GetKey(olc::Key::Q).bHeld)
        {
            fScaleX *= 1.001f;
            fScaleY *= 1.001f;
        }
        if (GetKey(olc::Key::A).bHeld)
        {
            fScaleX /= 1.001f;
            fScaleY /= 1.001f;
        }

        // now, calculate position after zoom and change offset
        float fMouseWorldX_after, fMouseWorldY_after;
        ScreenToWorld(fMouseX, fMouseY, fMouseWorldX_after, fMouseWorldY_after);
        fOffsetX += fMouseWorldX_before - fMouseWorldX_after;
        fOffsetY += fMouseWorldY_before - fMouseWorldY_after;

        // Display 
        Clear(olc::BLACK);

        // Clip
        float fWorldLeft, fWorldTop, fWorldRight, fWorldBottom;
        ScreenToWorld(0, 0, fWorldLeft, fWorldTop);
        ScreenToWorld(ScreenWidth(), ScreenHeight(), fWorldRight, fWorldBottom);

        // 10 horizontal and vertical lines
        int nLinesDrawn = 0;
        for (float y = 0.0f; y <= 10.0f; y++)
        {
            if (y >= fWorldTop && y <= fWorldBottom)
            {
                float sx = 0.0f, sy = y;
                float ex = 10.0f, ey = y;
                int pixel_sx, pixel_sy, pixel_ex, pixel_ey;
                WorldToScreen(sx, sy, pixel_sx, pixel_sy);
                WorldToScreen(ex, ey, pixel_ex, pixel_ey);
                DrawLine(pixel_sx, pixel_sy, pixel_ex, pixel_ey, olc::WHITE);
                nLinesDrawn++;
            }
        }
        for (float x = 0.0f; x <= 10.0f; x++)
        {
            if (x >= fWorldLeft && x <= fWorldRight)
            {
                float sx = x, sy = 0.0f;
                float ex = x, ey = 10.0f;
                int pixel_sx, pixel_sy, pixel_ex, pixel_ey;
                WorldToScreen(sx, sy, pixel_sx, pixel_sy);
                WorldToScreen(ex, ey, pixel_ex, pixel_ey);
                DrawLine(pixel_sx, pixel_sy, pixel_ex, pixel_ey, olc::WHITE);
                nLinesDrawn++;
            }
        }

        if (GetMouse(0).bPressed)
        {
            fSelectedCellX = (int)fMouseWorldX_after;
            fSelectedCellY = (int)fMouseWorldY_after;
        }

        // red circle at selected cell
        int cx, cy, cr;
        WorldToScreen(fSelectedCellX + 0.5f, fSelectedCellY + 0.5f, cx, cy);
        cr = 0.3f * fScaleX;
        FillCircle(cx, cy, cr, olc::RED);

        return true;
    }


};

int main()
{
    PanAndZoom app;
    if (app.Construct(160, 100, 4, 4))
		app.Start();

	return 0;
}