#include "Chunks.h"

Chunk::Chunk(string _path)
{
	path = _path;
}

Chunk::~Chunk()
{
	UnloadTexture(tex);
}

void Chunk::Update() //update anything inside chunk LATER
{
	//Chunks are static for now
}

void Chunk::Draw()
{
    // Draw the chunk texture
    DrawTexture(tex, x, y, WHITE);

    // Debug outline (red box around chunk)
    //DrawRectangleLines(x, y, tex.width, tex.height, RED);
}

void Chunk::Startup()
{
	cout << "Loading chunk from path: " << path << endl;
	tex = LoadTexture(path.c_str());
}

void Chunk::Shutdown()
{
	//Inform all entities inside chunk of unload LATER
	UnloadTexture(tex);
}

//-----------------Chunk Manager-----------------

ChunkManager::ChunkManager(EventManager& _eventmanager, DrawLayer& BGLayer) : eventmanager(_eventmanager), bglayer(BGLayer)
{
	eventmanager.AddSpecialListener("chunkmanager_listener", this, "PLAYER_POS_UPDATE");
	bglayer.AddDrawCall(this, 0); //Background layer at 0 priority

	//create all chunk objects in 20 wide 12 high grid for now
    for (int j = 0; j < 12; j++) {
        for (int i = 0; i < 20; i++) {
            int counter = j * 20 + i; // row-major index
            std::string path = "Levels/startlvl/chunks/Chunk_" + std::to_string(counter) + ".png";

            Chunk newchunk(path);
            newchunk.x = i * 768;
            newchunk.y = j * 768;
            chunkobjs.push_back(newchunk);
        }
    }
}

ChunkManager::~ChunkManager()
{
	eventmanager.RemoveSpecialListener("chunkmanager_listener");
}

void ChunkManager::OnSpecialEvent(std::string& command, std::vector<std::string> params)
{
    if (command == "PLAYER_POS_UPDATE")
    {
        if (params.size() < 2) return; // safety check

        int playerx = std::stoi(params[0]);
        int playery = std::stoi(params[1]);

        std::cout << "X: " << playerx << ", Y: " << playery << std::endl;

        int chunkSize = 768;   // pixels per chunk
        int loadRadiusX = 2;     // load 5 wide (2 left, 2 right, plus center)
        int loadRadiusY = 1;     // load 3 tall (1 up, 1 down, plus center)

        // Player’s current chunk indices
        int player_chunk_x = playerx / chunkSize;
        int player_chunk_y = playery / chunkSize;

        std::cout << "Active chunks: ";
        for (Chunk* c : activechunks) {
            std::cout << c->x / 768 << "," << c->y / 768 << "  ";
        }
        std::cout << std::endl;

        // Copy current active chunks to toremove
        toremove = activechunks;
        activechunks.clear();

        for (Chunk& chunk : chunkobjs)
        {
            int chunk_x = chunk.x / chunkSize;
            int chunk_y = chunk.y / chunkSize;

            // Symmetric check: is this chunk within radius of player?
            if (std::abs(chunk_x - player_chunk_x) <= loadRadiusX &&
                std::abs(chunk_y - player_chunk_y) <= loadRadiusY)
            {
                activechunks.push_back(&chunk);

                auto it = std::find(toremove.begin(), toremove.end(), &chunk);
                if (it != toremove.end())
                {
                    // Already active, remove from toremove
                    toremove.erase(it);
                }
                else
                {
                    // Newly active
                    chunk.Startup();
                }
            }
        }

        // Shut down chunks that are no longer active
        for (Chunk* chunk : toremove)
        {
            chunk->Shutdown();
        }
    }
}

void ChunkManager::Update()
{
	for (Chunk* chunk : activechunks)
	{
		chunk->Update();
	}
}

void ChunkManager::Draw()
{
	for (Chunk* chunk : activechunks)
	{
		chunk->Draw();
	}
}