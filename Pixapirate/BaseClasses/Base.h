#pragma once
#include <string>
#include <vector>
#include "raylib.h"


using namespace std;
// To store diff classes in same array

struct Node {
	int x, y;
	float gCost, hCost;
	Node* parent;
	float fCost() const { return gCost + hCost; }
};

class HasCollider {};
class Load
{
public:
	virtual void Draw() = 0;
};

class Listener
{
public:
	virtual void OnEvent(string& command) = 0;

	virtual void OnSpecialEvent(string& command, vector<string> params) {};
};

class ChunkObject
{
public:
	virtual void Update() = 0;
	virtual void PlayerNear() = 0;
	virtual void PlayerFar() = 0;

	virtual void Startup() = 0;
	virtual void Shutdown() = 0;
};


struct AnimationFrames
{
	vector<Texture2D> run;
	vector<Texture2D> idle;
};

struct listenerload
{
	Listener* listener;
	string command;
};

enum NPCState
{
	NPC_NORMAL, //Patrolling for patrol npcs, idle for stationary npcs
	NPC_SCARED, //Flee
	NPC_AGGRESSIVE, //For future use
	NPC_IDLE //For npcs that have been triggered to stop moving
};

class BaseNPC : public Load, public Listener, public HasCollider, public ChunkObject
{
protected:
	Vector2 chunkpos;

	string name;
	int health;

	int cframe = 0;
	float lasttime = 0.0f;
	int facingright = 1;
	NPCState state = NPCState::NPC_NORMAL;

	AnimationFrames hatframes;
	AnimationFrames headframes;
	AnimationFrames torsoframes;
	AnimationFrames armsframes;
	AnimationFrames legsframes;

	vector<AnimationFrames*> bodyframes = { &legsframes, &armsframes, &torsoframes, &headframes, &hatframes };
	vector<AnimationFrames*> draworder = { &torsoframes, &legsframes, &headframes, &armsframes, &hatframes };
public:
	Vector2 position;

	virtual void OnEvent(string& command) override = 0; // For eventmanager
	virtual void OnSpecialEvent(string& command, vector<string> params) override {}; //For player pos updates

	virtual void Draw() override = 0; //Called by chunk
	virtual void Update() override = 0; // Called by chunk
	virtual void PlayerNear() override = 0;
	virtual void PlayerFar() override = 0;

	virtual void Shutdown() override = 0; //If chunk is about to be unloaded
	virtual void Startup() override = 0; //If chunk is loaded
};