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
#include "Engine/Pathfinding.h"


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

	vector<Vector2> patrolpoints;
	vector<Vector2> currentpath;
	int currentpatrolpoint = 0;

	bool patrol;
	bool lawman;

	int point = 0;

	bool catchup = false; //only ticked to true if shutdown
	double shutdowntime = 0;
	double timeFrozen = 0.0;
};

class LandPatrolHivemind; //Forward declaration

class LandPatroler : public BaseNPC //nothing more that a foot soldier that is controller by LandPatrolHivemind
{
public:
	LandPatroler(int id, Vector2 position, int health, LandPatrolHivemind* hivemindptr); //Does not need to do anything but follow path and attack if close to player (controlled by hivemind)

	~LandPatroler();

	void UpdateFrame();

	void OnEvent(string& command) override {};// For eventmanager
	void OnSpecialEvent(string& command, vector<string> params) override {}; //For player pos updates

	void Draw() override; //Called by chunk
	void Update() override; // Called by chunk
	void PlayerNear() override;
	void PlayerFar() override;

	void Shutdown() override; //If chunk is about to be unloaded
	void Startup() override; //If chunk is loaded

	int point = 0;
	vector<Vector2> path; //set by hive mind (this npc just blindly follows whatever path it is given)
	LandPatrolHivemind* hivemindpointer; // if it reaches target and needs new directions
};

class LandPatrolHivemind : public BaseNPC//Controls a group of LandPatroler npcs to move together toward a target (The brains) also controls the draws, updates etc of its minions (only BaseNPC for chunks)
{
public:
	//has two types, chase and ambush
	LandPatrolHivemind(int difficulty, int size, bool Isambush, DrawLayer& entitylayer, EventManager& _playerposmanager, ChunkManager& _chunkmanager); //Difficulty affects health and damage, size affects number of minions spawned
	~LandPatrolHivemind(); //delete all minions


	void UpdateFrame(); //Never used but needed for BaseNPC

	void OnEvent(string& command) override {};// For eventmanager needed for eventmanager 
	void OnSpecialEvent(string& command, vector<string> params) override; //For player pos updates

	void Draw() override; //Called by chunk (useless)
	void Update() override; // Called by chunk (Updates minions)
	void PlayerNear() override; //does not care
	void PlayerFar() override; //does not care

	//Unused. If they are too far, simple destroy
	void Shutdown() override; //If chunk is about to be unloaded (unused since it is persistant)
	void Startup() override; //If chunk is loaded (unused since it is persistant)
private:
	Color debugcolors[6] = {RED,ORANGE, YELLOW, GREEN, BLUE, PURPLE};
	void DebugSpawnHitPatrols();
	void AssignPatrolers();
	DrawLayer& entitylayer;
	EventManager& playerposmanager;
	ChunkManager& chunkmanager;

	vector<LandPatroler*> minions;
	vector<Vector2> spawnpoints;
	vector<std::vector<Vector2>> paths;
	int difficulty;
	int size;
	bool isambush;

	Vector2 playerforwardvector;
	Vector2 playerpos;
};