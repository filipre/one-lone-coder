#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <iostream>
#include <string>

struct sBall
{
    float px, py;
    float vx, vy;
    float ax, ay;
    float radius;

    int id;
};

class CirclePhysics : public olc::PixelGameEngine
{
public:
    CirclePhysics()
    {
        sAppName = "Balls";
    }

private:
    // TODO: use real balls later
    // std::vector<std::pair<float, float>> modelCircle;
    std::vector<sBall> vecBalls;
    // sBall *pSelectedBall = nullptr;

    void AddBall(float x, float y, float r = 5.0f)
    {
        sBall b;
        b.px = x; b.py = y;
        b.vx = 0; b.vy = 0;
        b.ax = 0; b.ay = 0;
        b.radius = r;

        b.id = vecBalls.size();
        vecBalls.emplace_back(b);
    }

public:
    bool OnUserCreate() override
    {
        // modelCircle.push_back({ 0.0f, 0.0f });
        // int nPoints = 20;
        // for (int i = 0; i < nPoints; i++)
        //     modelCircle.push_back({ cosf(i / (float)(nPoints - 1) * 2.0f * 3.14159f), sinf(i / (float)(nPoints - 1) * 2.0f * 3.14159f) });

        float fDefaultRad = 8.0f;

        for (int i = 0; i < 10; i++)
            AddBall(rand() % ScreenWidth(), rand() % ScreenHeight(), rand() % 16 + 2);

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override 
    {
        Clear(olc::BLACK);

        // Draw Balls
        for (auto ball : vecBalls)
            DrawCircle(ball.px, ball.py, ball.radius, olc::WHITE);

        return true;
    }
};

int main()
{
    CirclePhysics balls;
    if (balls.Construct(160, 120, 8, 8))
		balls.Start();

	return 0;
}