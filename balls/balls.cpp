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
    float mass;

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
    sBall *pSelectedBall = nullptr;

    void AddBall(float x, float y, float r = 5.0f)
    {
        sBall b;
        b.px = x; b.py = y;
        b.vx = 0; b.vy = 0;
        b.ax = 0; b.ay = 0;
        b.radius = r;
        b.mass = r * 10.0f;

        b.id = vecBalls.size();
        vecBalls.emplace_back(b);
    }

public:
    bool OnUserCreate() override
    {
        // float fDefaultRad = 8.0f;
        // AddBall(ScreenWidth() * 0.25f, ScreenHeight() * 0.5f, fDefaultRad);
        // AddBall(ScreenWidth() * 0.75f, ScreenHeight() * 0.5f, fDefaultRad);

        for (int i = 0; i < 10; i++)
            AddBall(rand() % ScreenWidth(), rand() % ScreenHeight(), rand() % 16 + 2);

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override 
    {
        auto DoCirclesOverlap = [](float x1, float y1, float r1, float x2, float y2, float r2)
        {
            return fabs((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) <= (r1+r2)*(r1+r2);
        };

        auto IsPointInCircle = [](float x1, float y1, float r1, float px, float py)
        {
            return fabs((x1-px)*(x1-px) + (y1-py)*(y1-py)) < (r1*r1);
        };

        // Drag'n'Drop
        if (GetMouse(0).bPressed || GetMouse(1).bPressed)
        {
            pSelectedBall = nullptr;
            for (auto &ball : vecBalls)
            {
                if (IsPointInCircle(ball.px, ball.py, ball.radius, GetMouseX(), GetMouseY()))
                {
                    pSelectedBall = &ball;
                    break;
                }
            }
        }
        if (GetMouse(0).bHeld)
        {
            if (pSelectedBall != nullptr)
            {
                pSelectedBall->px = GetMouseX();
                pSelectedBall->py = GetMouseY();
                pSelectedBall->vx = 0;
                pSelectedBall->vy = 0;
            }
        }
        if (GetMouse(0).bReleased)
        {
            pSelectedBall = nullptr;
        }
        if (GetMouse(1).bReleased)
        {
            if (pSelectedBall != nullptr)
            {
                // Apply velocity
                pSelectedBall->vx = 5.0f * ((pSelectedBall->px) - (float)GetMouseX());
                pSelectedBall->vy = 5.0f * ((pSelectedBall->py) - (float)GetMouseY());
            }
            pSelectedBall = nullptr;
        }

        std::vector<std::pair<sBall*, sBall*>> vecCollidingPairs;

        // Update Ball Positions
        for (auto &ball : vecBalls)
        {
            // Drag
            ball.ax = ball.vx * -0.8f;
            ball.ay = ball.vy * -0.8f;

            // Update ball physics
            ball.vx += ball.ax * fElapsedTime;
            ball.vy += ball.ay * fElapsedTime;
            ball.px += ball.vx * fElapsedTime;
            ball.py += ball.vy * fElapsedTime;

            // Wrap balls around screen
            if (ball.px < 0)                ball.px += (float)ScreenWidth();
            if (ball.px >= ScreenWidth())   ball.px -= (float)ScreenWidth();
            if (ball.py < 0)                ball.py += (float)ScreenHeight();
            if (ball.py >= ScreenHeight())  ball.py -= (float)ScreenHeight();

            // Clamp velocity near zero
            if (fabs(ball.vx*ball.vx + ball.vy*ball.vy) < 0.01f)
            {
                ball.vx = 0;
                ball.vy = 0;
            }
        }

        // Static Resolution
        for (auto &ball : vecBalls)
        {
            for (auto &target : vecBalls)
            {
                if (ball.id != target.id)
                {
                    if (DoCirclesOverlap(ball.px, ball.py, ball.radius, target.px, target.py, target.radius))
                    {
                        vecCollidingPairs.push_back({ &ball, &target });

                        float fDistance = sqrtf((ball.px-target.px)*(ball.px-target.px) + (ball.py-target.py)*(ball.py-target.py));
                        float fOverlap = 0.5f * (fDistance - ball.radius - target.radius);
                        ball.px -= fOverlap * (ball.px - target.px) / fDistance;
                        ball.py -= fOverlap * (ball.py - target.py) / fDistance;
                        target.px += fOverlap * (ball.px - target.px) / fDistance;
                        target.py += fOverlap * (ball.py - target.py) / fDistance;
                    }
                }
            }
        }

        // Dynamic Collisions
        for (auto c : vecCollidingPairs)
        {
            sBall *b1 = c.first;
            sBall *b2 = c.second;

            float fDistance = sqrtf((b1->px - b2->px)*(b1->px - b2->px) + (b1->py - b2->py)*(b1->py - b2->py));

            // Normal
            float nx = (b2->px - b1->px) / fDistance;
            float ny = (b2->py - b1->py) / fDistance;

            // Tangent
            float tx = -ny;
            float ty = nx;

            // // Dot Product <velocity, tangent>
            float dpTan1 = b1->vx * tx + b1->vy * ty;
            float dpTan2 = b2->vx * tx + b2->vy * ty;

            // Dot Product <velocity, normal>
            float dpNorm1 = b1->vx * nx + b1->vy * ny;
            float dpNorm2 = b2->vx * nx + b2->vy * ny;

            // Conservation of momentum in 1D (complicated ...)
            float m1 = (dpNorm1 * (b1->mass - b2->mass) + 2.0f * b2->mass * dpNorm2) / (b1->mass + b2->mass);
            float m2 = (dpNorm2 * (b2->mass - b1->mass) + 2.0f * b1->mass * dpNorm1) / (b1->mass + b2->mass);
        
            // update ball velocities
            b1->vx = tx * dpTan1 + nx * m1;
            b1->vy = ty * dpTan1 + ny * m1;
            b2->vx = tx * dpTan2 + nx * m2;
            b2->vy = ty * dpTan2 + ny * m2;

            // float kx = (b1->vx - b2->vx);
			// float ky = (b1->vy - b2->vy);
			// float p = 2.0 * (nx * kx + ny * ky) / (b1->mass + b2->mass);
			// b1->vx = b1->vx - p * b2->mass * nx;
			// b1->vy = b1->vy - p * b2->mass * ny;
			// b2->vx = b2->vx + p * b1->mass * nx;
			// b2->vy = b2->vy + p * b1->mass * ny;
        }

        // Display
        Clear(olc::BLACK);

        // Draw Balls
        for (auto ball : vecBalls)
            DrawCircle(ball.px, ball.py, ball.radius, olc::WHITE);

        // Draw Static collision
        for (auto c : vecCollidingPairs)
            DrawLine(c.first->px, c.first->py, c.second->px, c.second->py, olc::RED);

        // Draw Cue
        if (GetMouse(1).bHeld && pSelectedBall != nullptr)
            DrawLine(pSelectedBall->px, pSelectedBall->py, GetMouseX(), GetMouseY(), olc::BLUE);

        return true;
    }
};

int main()
{
    CirclePhysics balls;
    if (balls.Construct(160, 120, 8/2, 8/2))
		balls.Start();

	return 0;
}