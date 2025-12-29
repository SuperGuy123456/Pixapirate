#pragma once

#include "raylib.h"
#include "../BaseClasses/Base.h"
#include "DrawingPipeline.h"
#include "EventManager.h"
#include <string>
#include <vector>

class Chunk : public Listener, public HasCollider
{
public:
    int gridX = 0;
    int gridY = 0;
    int worldX = 0;
    int worldY = 0;

    std::string path;
    Texture2D tex;
    vector<BaseNPC*> NPClist; //To draw and update with chunk and to handle stuff like player near and player far

    // ✅ NEW: 16x16 collision grid
    int collision[16][16];

    Chunk(std::string path, int gx, int gy, DrawLayer& entitylayer);
    ~Chunk();

    void Startup();
    void Shutdown();

    void Update(); // calls npc update
    void Draw();// only draws the chunks

    void OnEvent(std::string& command) override {};

	bool IsWalkableWorld(int wx, int wy);
    bool IsWalkableTile(int tx, int ty);

    void RemoveNPC(BaseNPC* npcpointer);
    void AddNPC(BaseNPC* npcpointer);
private:
	DrawLayer& entitylayer;
};

class ChunkManager : public Load, public Listener
{
public:
    Camera2D camera;
    static const int CHUNK_W = 20;
    static const int CHUNK_H = 12;
    static const int CHUNK_SIZE = 768;

    std::vector<std::vector<Chunk>> chunkGrid;
    std::vector<Chunk*> activeChunks;
    std::vector<Chunk*> toRemove;

    vector<BaseNPC*> Allnpcs; //Every npc ingame (for bookkeeping)
    vector<BaseNPC*> Persistantnpcs; //For NPCs not tied to a chunk

    ChunkManager(EventManager& eventmanager, DrawLayer& bgLayer, DrawLayer& entitylayer);
    ~ChunkManager();

    void Update();
    void Draw() override;

    void OnEvent(std::string& command) override {};
    void OnSpecialEvent(std::string& command, std::vector<std::string> params) override;

	bool IsWalkableAt(int wx, int wy);

	//Be careful when using as it can cause duplicates if not removed from previous chunk
	void AddNPCToChunk(BaseNPC* npcpointer, Vector2 chunk); //Uses add function from Chunk by looking up chunk using coords (DOES NOT REMOVE FROM ANYWHERE)
    void MarkPersistantWithoutRemove(BaseNPC* npcpointer); //simple adds to persistant list (DOES NOT REMOVE FROM ANYWHERE)

    void MarkPersistant(BaseNPC* npcpointer, Vector2 chunk); //Uses remove function from Chunk by looking up chunk using coords and adds to its persistant list
    void RemovePersistant(BaseNPC* npcpointer, Vector2 chunk); //removes from persistant and adds to chunk using Add method
    void ChangeChunks(BaseNPC* npcpointer, Vector2 oldchunk, Vector2 newchunk); // uses remove of old chunk and calls add of new chunk

    //Helper function for anything that need sto know what chunk it is in
    Vector2 GetCurrentChunk(Vector2 position)
    {
        int chunkX = static_cast<int>(position.x) / CHUNK_SIZE;
        int chunkY = static_cast<int>(position.y) / CHUNK_SIZE;
        return Vector2{ static_cast<float>(chunkX), static_cast<float>(chunkY) };
	}
private:
    EventManager& eventmanager;
    DrawLayer& bglayer;
	DrawLayer& entitylayer;

    // ✅ NEW: giant collision map
    std::vector<std::vector<int>> worldCollision;

    // ✅ NEW: loaders
    void LoadWorldCollision(const std::string& path);
    void AssignCollisionToChunks();

	bool IsRectWalkable(Rectangle r);
};