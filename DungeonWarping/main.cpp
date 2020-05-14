#define OLC_PGE_APPLICATION
#include "../olcPixelGameEngine.h"

class olcDungeon : public olc::PixelGameEngine
{
public:
    olcDungeon()
    {
        sAppName = "Dungeon Explorer";
    }

    struct Renderable
    {
        olc::Sprite* sprite = nullptr;
        olc::Decal* decal = nullptr;

        Renderable() { }

        void Load(const std::string& sFile)
        {
            sprite = new olc::Sprite(sFile);
            decal = new olc::Decal(sprite);
        }

        ~Renderable()
        {
            delete decal;
            delete sprite;
        }
    };

    struct vec3d
    {
        float x, y, z;
    };

    struct sQuad
    {
        vec3d points[4];
        olc::vf2d tile;
    };

    struct sCell
    {
        bool wall = false;
        olc::vi2d id[6]{ };
    };

    class World
    {
    public:
        olc::vi2d size;

    private:
        std::vector<sCell> vCells;
        sCell NullCell;

    public:
        World() { }

        void Create(int w, int h)
        {
            size = { w, h };
            vCells.resize(w * h);
        }

        sCell& GetCell(const olc::vi2d& v)
        {
            if (v.x >= 0 && v.x < size.x && v.y >= 0 && v.y < size.y)
                return vCells[v.y * size.x + v.x];
            else
                return NullCell;
        }
    };

    World world;
    Renderable rendAllWalls;

    olc::vf2d vCameraPos = { 0.0f, 0.0f };
    float fCameraAngle = 0.0f;
    float fCameraPitch = 5.5f;
    float fCameraZoom = 16.0f;

    olc::vi2d vTileSize = { 16, 16 };

    enum Face
    {
        Floor = 0,
        North = 1,
        East = 2,
        South = 3,
        West = 4,
        Top = 5
    };

public:
    bool OnUserCreate() override
    {
        rendAllWalls.Load("minecraft_sprites.png");

        world.Create(64, 64);

        for (int y=0; y<world.size.y; y++)
            for (int x=0; x<world.size.x; x++)
            {
                world.GetCell({x, y}).wall = false;
                world.GetCell({x, y}).id[Face::Floor] = olc::vi2d{ 0, 0} * vTileSize;
                world.GetCell({x, y}).id[Face::Top] = olc::vi2d{ 4, 0} * vTileSize;
                world.GetCell({x, y}).id[Face::North] = olc::vi2d{ 4, 0} * vTileSize;
                world.GetCell({x, y}).id[Face::South] = olc::vi2d{ 4, 0} * vTileSize;
                world.GetCell({x, y}).id[Face::West] = olc::vi2d{ 4, 0} * vTileSize;
                world.GetCell({x, y}).id[Face::East] = olc::vi2d{ 4, 0} * vTileSize;
            }

        return true;
    }

    std::array<vec3d, 8> CreateCube(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera)
    {
        // unit cube
        std::array<vec3d, 8> unitCube;
        unitCube[0] = { 0.0f, 0.0f, 0.0f };
        unitCube[1] = { fScale, 0.0f, 0.0f };
        unitCube[2] = { fScale, -fScale, 0.0f };
        unitCube[3] = { 0.0f, -fScale, 0.0f };
        unitCube[4] = { 0.0f, 0.0f, fScale };
        unitCube[5] = { fScale, 0.0f, fScale };
        unitCube[6] = { fScale, -fScale, fScale };
        unitCube[7] = { 0.0f, -fScale, fScale };

        // Translate cube in x-z plane
        for (int i=0; i<8; i++)
        {
            unitCube[i].x += (vCell.x * fScale - vCamera.x);
            unitCube[i].y += -vCamera.y;
            unitCube[i].z += (vCell.y * fScale - vCamera.z);
        }

        // Rotate cube in y-axis around origin
        std::array<vec3d, 8> rotCube;
        float s = sin(fAngle);
        float c = cos(fAngle);
        for (int i=0; i<8; i++)
        {
            rotCube[i].x = unitCube[i].x * c + unitCube[i].z * s;
            rotCube[i].y = unitCube[i].y;
            rotCube[i].z = unitCube[i].x * -s + unitCube[i].z * c;
        }

        // Rotate cube in x-axis around origin (tilting)
        std::array<vec3d, 8> worldCube;
        s = sin(fPitch);
        c = cos(fPitch);
        for (int i=0; i<8; i++)
        {
            worldCube[i].x = rotCube[i].x;
            worldCube[i].y = rotCube[i].y * c - rotCube[i].z * s;
            worldCube[i].z = rotCube[i].y * s + rotCube[i].z * c;
        }

        // Project cube orthographically
        std::array<vec3d, 8> projCube;
        for (int i=0; i<8; i++)
        {
            projCube[i].x = worldCube[i].x + ScreenWidth() * 0.5f;
            projCube[i].y = worldCube[i].y + ScreenHeight() * 0.5f;
            projCube[i].z = worldCube[i].z;
        }

        return projCube;
    }

    void GetFaceQuads(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera, std::vector<sQuad> &render)
    {
        std::array<vec3d, 8> projCube = CreateCube(vCell, fAngle, fPitch, fScale, vCamera);

        auto& cell = world.GetCell(vCell);

        auto MakeFace = [&](int v1, int v2, int v3, int v4, Face f)
        {
            render.push_back({ projCube[v1], projCube[v2], projCube[v3], projCube[v4], cell.id[f] });
        };

        if (!cell.wall)
        {
            MakeFace(4, 0, 1, 5, Face::Floor);
        }
        else
        {
            MakeFace(3, 0, 1, 2, Face::South);
            MakeFace(6, 5, 4, 7, Face::North);
            MakeFace(7, 4, 0, 3, Face::East);
            MakeFace(2, 1, 5, 6, Face::West);
            MakeFace(7, 3, 2, 6, Face::Top);
        }
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        std::vector<sQuad> vQuads;
        for (int y=0; y<world.size.y; y++)
            for (int x=0; x<world.size.x; x++)
                GetFaceQuads({ x, y }, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);

        Clear(olc::WHITE);
        // for (auto& q : vQuads)
            // TODO
            // rendAllWalls.decal
            // {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y}
            // q.tile
            // vTileSize

        return true;
    }
};

int main()
{
    olcDungeon demo;
    if (demo.Construct(640, 480, 1, 1, false))
        demo.Start();
    return 0;
}