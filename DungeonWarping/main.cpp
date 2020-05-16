#define OLC_PGE_APPLICATION
#include "../olcPixelGameEngine.h"

// see https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_DungeonWarping.cpp

class olcDungeon : public olc::PixelGameEngine
{
public:
    olcDungeon()
    {
        sAppName = "Dungeon Explorer";
    }

    struct Renderable
	{
		Renderable() {}

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

		olc::Sprite* sprite = nullptr;
		olc::Decal* decal = nullptr;
	};


    struct vec3d
    {
        float x, y, z;
    };

    struct sQuad
    {
        vec3d points[4];
        olc::vf2d tile; // TODO: check if vi2d also works, might be even more fitting
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
    Renderable rendSelect;
    Renderable rendAllWalls;

    olc::vf2d vCameraPos = { 0.0f, 0.0f };
    float fCameraAngle = 0.0f;
    float fCameraPitch = 5.5f;
    float fCameraZoom = 16.0f;

    bool bVisible[6];

    olc::vi2d vCursor = { 0, 0 };
	olc::vi2d vTileCursor = { 0,0 };
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
        rendSelect.Load("./DungeonWarping/highlight.png");
        rendAllWalls.Load("./DungeonWarping/minecraft_sprites.png");

        world.Create(20, 20);

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

    void CalculateVisibleFaces(std::array<vec3d, 8>& cube)
    {
        auto CheckNormal = [&](int v1, int v2, int v3)
        {
            olc::vf2d a = { cube[v1].x, cube[v1].y };
            olc::vf2d b = { cube[v2].x, cube[v2].y };
            olc::vf2d c = { cube[v3].x, cube[v3].y };
            return (b-a).cross(c-a) > 0;
        };

        bVisible[Face::Floor] = CheckNormal(4, 0, 1);
        bVisible[Face::South] = CheckNormal(3, 0, 1);
        bVisible[Face::North] = CheckNormal(6, 5, 4);
        bVisible[Face::East] = CheckNormal(7, 4, 0);
        bVisible[Face::West] = CheckNormal(2, 1, 5);
        bVisible[Face::Top] = CheckNormal(7, 3, 2);
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
            if (bVisible[Face::Floor]) MakeFace(4, 0, 1, 5, Face::Floor);
        }
        else
        {
            if (bVisible[Face::South]) MakeFace(3, 0, 1, 2, Face::South);
            if (bVisible[Face::North]) MakeFace(6, 5, 4, 7, Face::North);
            if (bVisible[Face::East]) MakeFace(7, 4, 0, 3, Face::East);
            if (bVisible[Face::West]) MakeFace(2, 1, 5, 6, Face::West);
            if (bVisible[Face::Top]) MakeFace(7, 3, 2, 6, Face::Top);
        }
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        // Input
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

        // Tile selection
		if (GetKey(olc::Key::TAB).bHeld)
		{
            Clear(olc::BLACK);
			DrawSprite({ 0, 0 }, rendAllWalls.sprite);
			DrawRect(vTileCursor * vTileSize, vTileSize);
			if (GetMouse(0).bPressed) vTileCursor = vMouse / vTileSize;
			return true;
		}

        // Camera
        if (GetKey(olc::Key::W).bHeld) fCameraPitch += 1.0f * fElapsedTime;
        if (GetKey(olc::Key::S).bHeld) fCameraPitch -= 1.0f * fElapsedTime;
        if (GetKey(olc::Key::D).bHeld) fCameraAngle += 1.0f * fElapsedTime;
        if (GetKey(olc::Key::A).bHeld) fCameraAngle -= 1.0f * fElapsedTime;
        if (GetKey(olc::Key::Q).bHeld) fCameraZoom += 5.0f * fElapsedTime;
        if (GetKey(olc::Key::E).bHeld) fCameraZoom -= 5.0f * fElapsedTime;

        // Apply textures
        if (GetKey(olc::Key::K1).bPressed) world.GetCell(vCursor).id[Face::North] = vTileCursor * vTileSize;
        if (GetKey(olc::Key::K2).bPressed) world.GetCell(vCursor).id[Face::East] = vTileCursor * vTileSize;
        if (GetKey(olc::Key::K3).bPressed) world.GetCell(vCursor).id[Face::South] = vTileCursor * vTileSize;
        if (GetKey(olc::Key::K4).bPressed) world.GetCell(vCursor).id[Face::West] = vTileCursor * vTileSize;
        if (GetKey(olc::Key::K5).bPressed) world.GetCell(vCursor).id[Face::Floor] = vTileCursor * vTileSize;
        if (GetKey(olc::Key::K6).bPressed) world.GetCell(vCursor).id[Face::Top] = vTileCursor * vTileSize;

        // Cursor
        if (GetKey(olc::Key::LEFT).bPressed) vCursor.x--;
        if (GetKey(olc::Key::RIGHT).bPressed) vCursor.x++;
        if (GetKey(olc::Key::UP).bPressed) vCursor.y--;
        if (GetKey(olc::Key::DOWN).bPressed) vCursor.y++;
        if (vCursor.x < 0) vCursor.x = 0;
        if (vCursor.y < 0) vCursor.y = 0;
        if (vCursor.x >= world.size.x) vCursor.x = world.size.x - 1;
        if (vCursor.y >= world.size.y) vCursor.y = world.size.y - 1;

        // Place blocks
        if (GetKey(olc::Key::SPACE).bPressed)
        {
            world.GetCell(vCursor).wall = !world.GetCell(vCursor).wall;
        }

        vCameraPos = { vCursor.x + 0.5f, vCursor.y + 0.5f };
        vCameraPos *= fCameraZoom;

        // Rendering

        std::array<vec3d, 8> cullCube = CreateCube({ 0, 0}, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y });
        CalculateVisibleFaces(cullCube);
        
        std::vector<sQuad> vQuads;
        for (int y=0; y<world.size.y; y++)
            for (int x=0; x<world.size.x; x++)
                GetFaceQuads({ x, y }, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);

        std::sort(vQuads.begin(), vQuads.end(), [](const sQuad& q1, const sQuad& q2)
        {
            float z1 = (q1.points[0].z + q1.points[1].z + q1.points[2].z + q1.points[3].z) * 0.25f;
            float z2 = (q2.points[0].z + q2.points[1].z + q2.points[2].z + q2.points[3].z) * 0.25f;
            return z1 < z2;
        });

        Clear(olc::WHITE);
        for (auto& q : vQuads)
        {
            // std::cout << "(" << q.points[0].x << ", " << q.points[0].y << ")" << std::endl;
            DrawPartialWarpedDecal
            (
                rendAllWalls.decal,
                { {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} },
                q.tile,
                vTileSize
            );
        }

        vQuads.clear();
        GetFaceQuads(vCursor, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);
        for (auto& q : vQuads)
            DrawWarpedDecal(rendSelect.decal, { {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} });

        return !GetKey(olc::Key::ESCAPE).bPressed;
    }
};

int main()
{
    olcDungeon demo;
    if (demo.Construct(257, 257, 3, 3, false))
        demo.Start();
    return 0;
}