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

enum ChaseStage
{
	APPROACH, //When going to player position
	BEGINFLANK, //When going to a position in front of player to initiate flank
	FLANKING //When going to a designated flank position
};

enum FlankRole
{
	ROLE_REAR,
	ROLE_SIDE_LEFT,
	ROLE_SIDE_RIGHT,
	ROLE_SUPPORT_TOP_LEFT,
	ROLE_SUPPORT_TOP_RIGHT,
	ROLE_SUPPORT_BOTTOM_LEFT,
	ROLE_SUPPORT_BOTTOM_RIGHT,
	ROLE_CUTOFF
};
class LandPatroler; //Forward declaration
struct TaskSlot
{
	FlankRole role;
	LandPatroler* assigned = nullptr;
};

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

	void HIVEMIND_StartShooting();
	void HIVEMIND_ChangeState(ChaseStage newstage);

	int point = 0;
	vector<Vector2> path; //set by hive mind (this npc just blindly follows whatever path it is given)
	LandPatrolHivemind* hivemindpointer; // if it reaches target and needs new directions

	ChaseStage task = ChaseStage::APPROACH;
	bool shoot = false;

	int id;

	float speed = 2.5f;
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

	void LANDPATROL_ReachedTarget(LandPatroler* minion); //Called by minions when they reach their target to get new orders

	vector<LandPatroler*> minions;
private:
	Color debugcolors[6] = {RED,ORANGE, YELLOW, GREEN, BLUE, PURPLE};
	void DebugSpawnHitPatrols();
	void AssignPatrolers();
	Vector2 ComputeStartFlankPoint(float playerSpeed,Vector2 enemyPos,float enemySpeed);
	float RandomFloat(float min, float max);
	void AssignFlankRoles(LandPatroler* minion);
	void CalculateFlankPoint(LandPatroler* minion, TaskSlot* slotgiven);

	DrawLayer& entitylayer;
	EventManager& playerposmanager;
	ChunkManager& chunkmanager;

	vector<Vector2> spawnpoints;
	vector<std::vector<Vector2>> paths;
	int difficulty;
	int size;
	bool isambush;

	Vector2 playerforwardvector;
	Vector2 lastforward = { 0 , 0 }; //in order to detect change and recaluculate
	Vector2 playerpossincelastforward = { 0 , 0 }; //set when forward changes
	Vector2 playerpos;

	vector<TaskSlot> taskboard;
	vector<Vector2> flankstarts;
};