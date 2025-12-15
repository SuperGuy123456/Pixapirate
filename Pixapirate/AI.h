#pragma once
#include "BaseClasses/Base.h"
#include "Engine/DrawingPipeline.h"
#include "Engine/EventManager.h"
#include "Engine/Spritesplitter.h"
#include "Engine/Allstructs.h"
#include "Engine/Chunks.h"
#include <iostream>
#include <vector>
#include <string>
#include "raymath.h"


std::vector<Vector2> FindPath(Vector2 start, Vector2 end, ChunkManager& cm);

class SimpleNPC : public BaseNPC
{
public:
	//For a simple npc that goes from point A to point B back to point A indefinatly unless triggered or scared by player
	SimpleNPC(string name, Vector2 position, int health, EventManager& _playerposmanager,ChunkManager& chunkmanager, const vector<Vector2>& patrolpoints, bool lawman);
	//For a simple npc that stands still until triggered (scared or hurt by player)
	SimpleNPC(string name, Vector2 position, int health, EventManager& _playerposmanager,ChunkManager& chunkmanager, bool lawman);


	~SimpleNPC();

	void UpdateFrame();

	void OnEvent(string& command) override {};// For eventmanager
	void OnSpecialEvent(string& command, vector<string> params) override {}; //For player pos updates

	void Draw() override; //Called by chunk
	void Update() override; // Called by chunk
	void PlayerNear() override;
	void PlayerFar() override;

	void Shutdown() override; //If chunk is about to be unloaded
	void Startup() override; //If chunk is loaded

private:
	EventManager& playerposmanager;
	ChunkManager& chunkmanager;

	Texture2D npcTexture;

	vector<Vector2> patrolpoints;
	vector<Vector2> currentpath;
	int currentpatrolpoint = 0;

	bool patrol;
	bool lawman;
};
