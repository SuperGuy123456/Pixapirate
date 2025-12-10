#pragma once

#include "raylib.h"
#include "../BaseClasses/Base.h"
#include "DrawingPipeline.h"
#include "EventManager.h"
#include <string>
#include <iostream>

class Chunk : public Listener, public HasCollider
{
public:
	int x = 0;
	int y = 0;
	string path;
	Texture2D tex;
	Chunk(string path);
	~Chunk();

	void Startup(); //When chunk is first loaded (load texture)
	void Shutdown(); //Right before chunk is unloaded (End of life such as informing all entities inside)

	void Update();
	void Draw();

	void OnEvent(string& command) override {};
};


class ChunkManager : public Load, public Listener
{
public:
	vector<Chunk> chunkobjs;
	vector<Chunk*> activechunks;
	vector<Chunk*> toremove;

	ChunkManager(EventManager& _eventmanager, DrawLayer& BGLayer);
	~ChunkManager();

	void Update(); //Will update all active chunks
	void Draw() override; //Will draw all active chunks

	void OnEvent(string& command) override {};
	void OnSpecialEvent(string& command, vector<string> params) override; // listenes for player pos update and when it does, updates active chunks

private:
	EventManager& eventmanager;
	DrawLayer& bglayer;
};