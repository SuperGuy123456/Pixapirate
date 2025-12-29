#include "Chunks.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

#pragma region ChunkSTUFF
// ---------------- Chunk ----------------

Chunk::Chunk(std::string _path, int gx, int gy, DrawLayer& _entitylayer) : entitylayer(_entitylayer)
{
    path = _path;
    gridX = gx;
    gridY = gy;

    worldX = gx * 768;
    worldY = gy * 768;
}

Chunk::~Chunk()
{
    UnloadTexture(tex);
}

void Chunk::Startup()
{
    std::cout << "Loading chunk: " << path << std::endl;
    tex = LoadTexture(path.c_str());

	for (BaseNPC* npc : NPClist)
    {
        npc->Startup();
		entitylayer.AddDrawCall(npc, npc->position.y); // NPCs drawn above chunk and based on y position
    }
}

void Chunk::Shutdown()
{
    UnloadTexture(tex);

    for (BaseNPC* npc : NPClist)
    {
        npc->Shutdown();
		entitylayer.RemoveDrawCall(npc);
    }
}

void Chunk::Update()
{
    for (BaseNPC* npc : NPClist)
    {
        npc->Update();
		entitylayer.ChangePriority(npc, npc->position.y); // Update draw priority based on y position
	}
}

void Chunk::Draw()
{
	DrawTexture(tex, worldX, worldY, WHITE); //does not have to draw npcs as they are in entity layer and are clled by DrawingPipline
}

// ✅ Convert world pixel → tile inside this chunk
bool Chunk::IsWalkableWorld(int wx, int wy)
{
    int tileX = (wx - worldX) / 48;
    int tileY = (wy - worldY) / 48;

    if (tileX < 0 || tileY < 0 || tileX >= 16 || tileY >= 16)
        return false;

    int v = collision[tileY][tileX];
    return (v == 4); // 4 = walkable
}

bool Chunk::IsWalkableTile(int tx, int ty)
{
    if (tx < 0 || ty < 0 || tx >= 16 || ty >= 16)
        return false;

    return (collision[ty][tx] == 4);
}

void Chunk::AddNPC(BaseNPC* npcpointer)
{
    NPClist.push_back(npcpointer);
    entitylayer.AddDrawCall(npcpointer, npcpointer->position.y); // NPCs drawn above chunk
}

void Chunk::RemoveNPC(BaseNPC* npcpointer)
{
    auto it = std::find(NPClist.begin(), NPClist.end(), npcpointer);
    if (it != NPClist.end())
    {
        NPClist.erase(it);
		entitylayer.RemoveDrawCall(npcpointer);
    }
}
#pragma endregion

#pragma region ChunkManagerSTUFF
// ---------------- Chunk Manager ----------------

ChunkManager::ChunkManager(EventManager& _eventmanager, DrawLayer& _bglayer, DrawLayer& _entitylayer)
    : eventmanager(_eventmanager), bglayer(_bglayer), entitylayer(_entitylayer)
{
    eventmanager.AddSpecialListener("chunkmanager_listener", this, "PLAYER_POS_UPDATE");
    bglayer.AddDrawCall(this, 0);

    // ✅ Allocate and construct the 2D chunk grid IN PLACE
    chunkGrid.resize(CHUNK_W);
    for (int x = 0; x < CHUNK_W; x++)
    {
        chunkGrid[x].reserve(CHUNK_H);
        for (int y = 0; y < CHUNK_H; y++)
        {
            int index = y * CHUNK_W + x;
            std::string path = "Levels/startlvl/chunks/Chunk_" + std::to_string(index) + ".png";

            // ✅ Construct chunk directly inside the vector (no assignment!)
            chunkGrid[x].emplace_back(path, x, y, entitylayer);
        }
    }

    // ✅ Load collision and slice into chunks
    LoadWorldCollision("Levels/startlvl/world_collision.txt");
    AssignCollisionToChunks();
}
ChunkManager::~ChunkManager()
{
    eventmanager.RemoveSpecialListener("chunkmanager_listener");
}

// ✅ Load collision rows (3840 rows, 16 columns each)
void ChunkManager::LoadWorldCollision(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Failed to load world collision file: " << path << std::endl;
        return;
    }

    worldCollision.clear();
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        std::vector<int> row;
        std::stringstream ss(line);
        int value;

        while (ss >> value)
            row.push_back(value);

        worldCollision.push_back(row);
    }

    std::cout << "Loaded world collision map ("
        << worldCollision.size() << " rows)" << std::endl;
}

// ✅ Slice 3840 rows → 240 chunks → 16 rows per chunk
void ChunkManager::AssignCollisionToChunks()
{
    const int rowsPerChunk = 16;
    const int totalChunks = CHUNK_W * CHUNK_H;

    if (worldCollision.size() < totalChunks * rowsPerChunk)
    {
        std::cout << "worldCollision too small! rows=" << worldCollision.size()
            << " expected at least=" << totalChunks * rowsPerChunk << std::endl;
        return;
    }

    for (int cy = 0; cy < CHUNK_H; cy++)
    {
        for (int cx = 0; cx < CHUNK_W; cx++)
        {
            int chunkIndex = cy * CHUNK_W + cx;
            Chunk& chunk = chunkGrid[cx][cy];

            int rowStart = chunkIndex * rowsPerChunk;

            for (int y = 0; y < 16; y++)
            {
                const std::vector<int>& srcRow = worldCollision[rowStart + y];

                for (int x = 0; x < 16; x++)
                {
                    if (x < (int)srcRow.size())
                        chunk.collision[y][x] = srcRow[x];
                    else
                        chunk.collision[y][x] = 4; // default walkable
                }
            }
        }
    }

    std::cout << "Assigned collision data to all chunks (per-chunk block mode)" << std::endl;
}

// ✅ World-space walkability check
bool ChunkManager::IsWalkableAt(int wx, int wy)
{
    int cx = wx / CHUNK_SIZE;
    int cy = wy / CHUNK_SIZE;

    if (cx < 0 || cy < 0 || cx >= CHUNK_W || cy >= CHUNK_H)
        return false;

    Chunk& chunk = chunkGrid[cx][cy];
    return chunk.IsWalkableWorld(wx, wy);
}

void ChunkManager::OnSpecialEvent(std::string& command, std::vector<std::string> params)
{
    if (command != "PLAYER_POS_UPDATE") return;
    if (params.size() < 2) return;

    int playerx = std::stoi(params[0]);
    int playery = std::stoi(params[1]);

    int px = playerx / CHUNK_SIZE;
    int py = playery / CHUNK_SIZE;

    int loadRadiusX = 2;
    int loadRadiusY = 1;

    toRemove = activeChunks;
    activeChunks.clear();

    for (int dx = -loadRadiusX; dx <= loadRadiusX; dx++)
    {
        for (int dy = -loadRadiusY; dy <= loadRadiusY; dy++)
        {
            int cx = px + dx;
            int cy = py + dy;

            if (cx < 0 || cy < 0 || cx >= CHUNK_W || cy >= CHUNK_H)
                continue;

            Chunk* chunk = &chunkGrid[cx][cy];
            activeChunks.push_back(chunk);

            auto it = std::find(toRemove.begin(), toRemove.end(), chunk);
            if (it != toRemove.end())
            {
                toRemove.erase(it);
            }
            else
            {
                chunk->Startup();
            }
        }
    }

    for (Chunk* c : toRemove)
        c->Shutdown();
}

void ChunkManager::Update()
{
    // Update active chunks
    for (Chunk* c : activeChunks)
        c->Update();

    for (BaseNPC* npcpointer : Persistantnpcs)
    {
        npcpointer->Update();
    }

    // -----------------------------------------
    // ✅ DEBUG: Click to test collision
    // -----------------------------------------
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        // 1. Get screen-space mouse
        Vector2 screen = GetMousePosition();

        // 2. Convert to world-space
        Vector2 world = GetScreenToWorld2D(screen, camera);

        // 3. Convert to tile coords
        int tileX = (int)(world.x / 48);
        int tileY = (int)(world.y / 48);

        // 4. Run your collision check
        bool walkable = IsWalkableAt(world.x, world.y);

        // 5. Print everything
        std::cout << "---- MOUSE COLLISION DEBUG ----\n";
        std::cout << "Screen: (" << screen.x << ", " << screen.y << ")\n";
        std::cout << "World:  (" << world.x << ", " << world.y << ")\n";
        std::cout << "Tile:   (" << tileX << ", " << tileY << ")\n";
        std::cout << "Walkable? " << (walkable ? "YES" : "NO") << "\n";
        std::cout << "--------------------------------\n";
    }
}
void ChunkManager::Draw()
{
    for (Chunk* c : activeChunks)
        c->Draw();
}

bool ChunkManager::IsRectWalkable(Rectangle r)
{
    int leftTile = r.x / 48;
    int rightTile = (r.x + r.width) / 48;
    int topTile = r.y / 48;
    int bottomTile = (r.y + r.height) / 48;

    for (int ty = topTile; ty <= bottomTile; ty++)
    {
        for (int tx = leftTile; tx <= rightTile; tx++)
        {
            if (!IsWalkableAt(tx * 48, ty * 48))
                return false;
        }
    }

    return true;
}

void ChunkManager::MarkPersistant(BaseNPC* npcpointer, Vector2 chunk)
{
	Chunk* oldchunk = &chunkGrid[(int)chunk.x][(int)chunk.y];
	oldchunk->RemoveNPC(npcpointer);
	Persistantnpcs.push_back(npcpointer);
	entitylayer.AddDrawCall(npcpointer, npcpointer->position.y); // NPCs drawn above chunk
}
void ChunkManager::MarkPersistantWithoutRemove(BaseNPC* npcpointer)
{
    Persistantnpcs.push_back(npcpointer);
	entitylayer.AddDrawCall(npcpointer, npcpointer->position.y); // NPCs drawn above chunk
}

void ChunkManager::RemovePersistant(BaseNPC* npcpointer, Vector2 chunk)
{
	entitylayer.RemoveDrawCall(npcpointer);
    auto it = std::find(Persistantnpcs.begin(), Persistantnpcs.end(), npcpointer);
    if (it != Persistantnpcs.end())
    {
        Persistantnpcs.erase(it);
    }
    Chunk* newchunk = &chunkGrid[(int)chunk.x][(int)chunk.y];
    newchunk->AddNPC(npcpointer);
}

void ChunkManager::ChangeChunks(BaseNPC* npcpointer, Vector2 oldchunk, Vector2 newchunk)
{
    Chunk* ochunk = &chunkGrid[(int)oldchunk.x][(int)oldchunk.y];
    ochunk->RemoveNPC(npcpointer);
    Chunk* nchunk = &chunkGrid[(int)newchunk.x][(int)newchunk.y];
    nchunk->AddNPC(npcpointer);
}

void ChunkManager::AddNPCToChunk(BaseNPC* npcpointer, Vector2 chunk)
{
    Chunk* targetchunk = &chunkGrid[(int)chunk.x][(int)chunk.y];
    targetchunk->AddNPC(npcpointer);
}

#pragma endregion