#include "AI.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

#pragma region SimpleNPCSTUFF
SimpleNPC::SimpleNPC(string name, Vector2 position, int health, EventManager& _playerposmanager,ChunkManager& _chunkmanager, const vector<Vector2>& patrolpoints, bool lawman)
	:  playerposmanager(_playerposmanager), patrolpoints(patrolpoints), chunkmanager(_chunkmanager) //simple npcs technically only need player pos if the are fleeing
{
	this->name = name;
	this->position = position;
	this->health = health;

	SpriteSplitter::Clothing("Art/Hats/hat_placeholder_clean.png",
		"Art/Head/light/head_light_midbeard_clean.png",
		"Art/Torso/light/torso_light_sailor_clean.png",
		"Art/Arms/light/arms_light_simple_clean.png",
		"Art/Legs/light/legs_light_tanboots_used.png",
		hatframes, headframes, torsoframes, armsframes, legsframes); //Automatically sets all frames

	patrol = true; //This npc will patrol between points

	//Register with chunkmanager for persistant npc management
	chunkmanager.Allnpcs.push_back(this);

	chunkmanager.AddNPCToChunk(this, Vector2{ position.x / ChunkManager::CHUNK_SIZE, position.y / ChunkManager::CHUNK_SIZE });

	chunkpos = chunkmanager.GetCurrentChunk(position);

	if (patrol) { currentpath = BuildPatrolPath(patrolpoints, chunkmanager, 45, 72); }
}

SimpleNPC::SimpleNPC(string name, Vector2 position, int health, EventManager& _playerposmanager,ChunkManager& _chunkmanager, bool lawman)
	:  playerposmanager(_playerposmanager), chunkmanager(_chunkmanager)
{
	this->name = name;
	this->position = position;
	this->health = health;

	SpriteSplitter::Clothing("Art/Hats/hat_placeholder_clean.png",
		"Art/Head/light/head_light_midbeard_clean.png",
		"Art/Torso/light/torso_light_sailor_clean.png",
		"Art/Arms/light/arms_light_simple_clean.png",
		"Art/Legs/light/legs_light_tanboots_used.png",
		hatframes, headframes, torsoframes, armsframes, legsframes); //Automatically sets all frames (Test sailor outfit)

	patrol = false; //This npc will patrol between points
}

SimpleNPC::~SimpleNPC()
{
	for (AnimationFrames* bodypart : bodyframes)
	{
		for (Texture2D& runtex : bodypart->run)
		{
			UnloadTexture(runtex);
		}

		for (Texture2D& idletex : bodypart->idle)
		{
			UnloadTexture(idletex);
		}
	}
}

void SimpleNPC::UpdateFrame()
{
	if (GetTime() - lasttime >= 0.2)
	{
		lasttime = GetTime();
		if (state == NPCState::NPC_NORMAL and patrol)
		{
			if (cframe == 3) //if run prev played and ended in 4th frame, clamp to 0
			{
				cframe = 0;
			}
			else if (cframe == 3) //only has 3 frames
			{
				cframe = 0;
			}
			else
			{
				cframe++;
			}
		}
		else if (state == NPCState::NPC_NORMAL and patrol == false)
		{
			if (cframe == 2) //has 4 frames
			{
				cframe = 0;
			}
			else
			{
				cframe++;
			}
		}
	}
}

void SimpleNPC::Update()
{
	UpdateFrame();

	// -----------------------------
	// CATCH-UP LOGIC
	// -----------------------------
	if (catchup)
	{
		std::cout << "Shutdown at: " << shutdowntime << "\n";
		std::cout << "Startup at: " << GetTime() << "\n";
		std::cout << "timeFrozen = " << timeFrozen << "\n";

		float distance = 2.0f * timeFrozen * 60.0f; // your NPC speed

		while (distance > 0 && !currentpath.empty())
		{
			// ✅ Ensure point is always valid BEFORE using it
			if (point >= currentpath.size())
				point = 0;

			std::cout << "catching up... point=" << point
				<< " distance=" << distance << "\n";

			Vector2 next = currentpath[point];
			float distToNext = Vector2Distance(position, next);

			if (distToNext == 0)
			{
				// Already exactly on this waypoint → move to next
				point++;
				continue;
			}

			if (distance >= distToNext)
			{
				// ✅ NPC would have reached this waypoint
				position = next;
				distance -= distToNext;

				point++;
				if (point >= currentpath.size())
					point = 0;
			}
			else
			{
				// ✅ NPC would be partway to the next waypoint
				Vector2 dir = Vector2Normalize(Vector2Subtract(next, position));
				position.x += dir.x * distance;
				position.y += dir.y * distance;
				distance = 0;
			}
		}

		catchup = false;
	}

	// -----------------------------
	// NORMAL PATROL MOVEMENT
	// -----------------------------
	if (patrol)
	{
		if (!currentpath.empty())
		{
			if (point >= currentpath.size())
				point = 0;

			Vector2 next = currentpath[point];
			Vector2 dir = Vector2Normalize(Vector2Subtract(next, position));

			position.x += dir.x;
			position.y += dir.y;

			if (dir.x > 0) facingright = 1;
			else if (dir.x < 0) facingright = -1;

			if (fabs(position.x - next.x) < 2 && fabs(position.y - next.y) < 2)
			{
				position = next;

				point++;
				if (point >= currentpath.size())
					point = 0;
			}
		}

		Vector2 currentchunkcoords = chunkmanager.GetCurrentChunk(position);
		if (currentchunkcoords.x != chunkpos.x || currentchunkcoords.y != chunkpos.y)
		{
			chunkmanager.ChangeChunks(this, chunkpos, currentchunkcoords);
			chunkpos = currentchunkcoords;
		}
	}
}

void SimpleNPC::Draw()
{
	/*for (const Vector2& point : currentpath)
	{
		DrawCircleV(point, 5.0f, RED);
	}*/
	for (AnimationFrames* bodypart : draworder)
	{
		Texture2D tex;

		if (state == NPCState::NPC_IDLE)
		{
			if (cframe == 3)
			{
				cframe = 2; // Since run has 4 frames and idle only has 3
			}
			tex = bodypart->idle[cframe];
		}
		else
		{
			tex = bodypart->run[cframe];
		}

		// Source rectangle: full texture
		Rectangle src = { 0.0f, 0.0f, facingright * (float)tex.width, (float)tex.height };

		// Destination rectangle: position and size on screen
		Rectangle dest = { (float)position.x, (float)position.y, (float)tex.width, (float)tex.height };

		// Origin: pivot point (top-left here)
		Vector2 origin = { 0.0f, 0.0f };

		// Rotation: none
		float rotation = 0.0f;

		DrawTexturePro(tex, src, dest, origin, rotation, WHITE);
	}
}

void SimpleNPC::PlayerNear()
{
	// TEMP: do nothing
}

void SimpleNPC::PlayerFar()
{
	// TEMP: do nothing
}

void SimpleNPC::Shutdown()
{
	// TEMP: do nothing
	for (AnimationFrames* bodypart : bodyframes)
	{
		for (Texture2D runtex : bodypart->run)
		{
			UnloadTexture(runtex);
		}

		for (Texture2D idletex : bodypart->idle)
		{
			UnloadTexture(idletex);
		}
		bodypart->run.clear();   // ✅ IMPORTANT
		bodypart->idle.clear();  // ✅ IMPORTANT
	}
	shutdowntime = GetTime();
	catchup = true; //trying to see problem
}

void SimpleNPC::Startup()
{
	timeFrozen = GetTime() - shutdowntime;
	catchup = true;
	SpriteSplitter::Clothing("Art/Hats/hat_placeholder_clean.png",
		"Art/Head/light/head_light_midbeard_clean.png",
		"Art/Torso/light/torso_light_sailor_clean.png",
		"Art/Arms/light/arms_light_simple_clean.png",
		"Art/Legs/light/legs_light_tanboots_used.png",
		hatframes, headframes, torsoframes, armsframes, legsframes); //Automatically sets all frames
}

#pragma endregion

#pragma region LandPatrolerStuff
LandPatroler::LandPatroler(int id, Vector2 position, int health, LandPatrolHivemind* hivemindptr) : hivemindpointer(hivemindptr)
{
	this->position = position;
	this->health = health;

	SpriteSplitter::Clothing("Art/Hats/hat_placeholder_clean.png",
		"Art/Head/light/head_light_midbeard_clean.png",
		"Art/Torso/light/torso_light_sailor_clean.png",
		"Art/Arms/light/arms_light_simple_clean.png",
		"Art/Legs/light/legs_light_tanboots_used.png",
		hatframes, headframes, torsoframes, armsframes, legsframes); //Automatically sets all frames
}

LandPatroler::~LandPatroler()
{
	for (AnimationFrames* bodypart : bodyframes)
	{
		for (Texture2D& runtex : bodypart->run)
		{
			UnloadTexture(runtex);
		}
		for (Texture2D& idletex : bodypart->idle)
		{
			UnloadTexture(idletex);
		}
	}
}

void LandPatroler::UpdateFrame()
{
	if (GetTime() - lasttime >= 0.2)
	{
		lasttime = GetTime();

		if (state == NPCState::NPC_NORMAL)
		{
			// run has 4 frames: 0,1,2,3
			if (cframe >= hatframes.run.size()-1)
				cframe = 0;
			else
				cframe++;
		}
		else if (state == NPCState::NPC_IDLE)
		{
			// idle has 3 frames: 0,1,2
			if (cframe >= hatframes.idle.size()-1)
				cframe = 0;
			else
				cframe++;
		}
	}
}

void LandPatroler::Draw()
{
	for (AnimationFrames* bodypart : draworder)
	{
		Texture2D tex;
		if (state == NPCState::NPC_IDLE)
		{
			tex = bodypart->idle[cframe];
		}
		else
		{
			tex = bodypart->run[cframe];
		}
		// Source rectangle: full texture
		Rectangle src = { 0.0f, 0.0f, facingright * (float)tex.width, (float)tex.height };
		// Destination rectangle: position and size on screen
		Rectangle dest = { (float)position.x, (float)position.y, (float)tex.width, (float)tex.height };
		// Origin: pivot point (top-left here)
		Vector2 origin = { 0.0f, 0.0f };
		// Rotation: none
		float rotation = 0.0f;
		DrawTexturePro(tex, src, dest, origin, rotation, WHITE);
	}
}

void LandPatroler::Update()
{
	if (!path.empty())
	{
		Vector2 next = path[point];

		// Move toward next waypoint
		Vector2 dir = Vector2Normalize(Vector2Subtract(next, position));

		position.x += dir.x * 2.25f;
		position.y += dir.y * 2.5f;

		if (dir.x > 0)      facingright = 1;
		else if (dir.x < 0) facingright = -1;

		// Reached waypoint?
		if (fabs(position.x - next.x) < 2 && fabs(position.y - next.y) < 2)
		{
			position = next; // Snap

			// If this was the last waypoint, STOP
			if (point + 1 >= path.size())
			{
				// Stop moving by clearing the path
				path.clear();
				return;
			}

			// Otherwise continue to next waypoint
			point++;
		}
	}
	else
	{
		if (state != NPCState::NPC_IDLE)
		{
			cframe = 0;
		}
		state = NPCState::NPC_IDLE;
	}
	UpdateFrame();
}

void LandPatroler::PlayerNear() {};
void LandPatroler::PlayerFar() {};
void LandPatroler::Shutdown() {};
void LandPatroler::Startup() {};


#pragma endregion

#pragma region LandPatrolHivemindStuff
LandPatrolHivemind::LandPatrolHivemind(int difficulty, int size, bool Isambush, DrawLayer& _entitylayer, EventManager& _playerposmanager, ChunkManager& _chunkmanager) : entitylayer(_entitylayer), playerposmanager(_playerposmanager), chunkmanager(_chunkmanager)
{
	this->difficulty = difficulty;
	this->size = size;
	this->isambush = Isambush;

	if (isambush)
	{
		//the side -> front -> back encounter TODO
	}
	else //chase encounter
	{

	}
	chunkmanager.MarkPersistantWithoutRemove(this);
	playerposmanager.AddSpecialListener("hivemindlistener " + to_string(size) + " " + to_string(difficulty), this, "PLAYER_POS_UPDATE"); //For the eventmanager to call the OnSpecialEvent
}
LandPatrolHivemind::~LandPatrolHivemind()
{
	for (LandPatroler* minion : minions)
	{
		delete minion;
		entitylayer.RemoveDrawCall(minion);
	}
	minions.clear();
}

void LandPatrolHivemind::Update()
{
	for (LandPatroler* minion : minions)
	{
		minion->Update();
		entitylayer.ChangePriority(minion, static_cast<int>(minion->position.y)); //Update draw priority based on y position
	}

	if (IsKeyPressed(KEY_R))
	{
		this->DebugSpawnHitPatrols();
	}
}

void LandPatrolHivemind::OnSpecialEvent(string& command, vector<string> params)
{
	if (command != "PLAYER_POS_UPDATE") { return; }

	//if it was a player pos update

	//pos.x, pos.y, playerforwardvectorvector.x, playerforwardvectorvector.y (the params structure)
	playerpos = Vector2{ (float)stoi(params[0]),(float)stoi(params[1]) };
	playerforwardvector = Vector2{ (float)stoi(params[2]),(float)stoi(params[3]) };

}
inline float RandomFloat(float min, float max)
{
	return min + (float)GetRandomValue(0, 1000000) / 1000000.0f * (max - min);
}

void LandPatrolHivemind::DebugSpawnHitPatrols()
{
	spawnpoints.clear();
	paths.clear();

	// Normalize playerforwardvector
	if (playerforwardvector.x != 0 || playerforwardvector.y != 0)
		playerforwardvector = Vector2Normalize(playerforwardvector);
	else
		playerforwardvector = { 1, 0 };

	float playerAngle = atan2f(playerforwardvector.y, playerforwardvector.x);
	float baseAngle = playerAngle + PI; // behind the player

	float coneWidth = PI / 2.0f; // 90 degrees
	float r = 600.0f;

	// Spawn "size" patrolers
	for (int s = 0; s < size; s++)
	{
		Vector2 spawn = { 0, 0 };
		bool valid = false;

		// 1. Try center of cone first
		{
			float angle = baseAngle;

			spawn = {
				playerpos.x + r * cosf(angle),
				playerpos.y + r * sinf(angle)
			};

			if (chunkmanager.IsWalkableAt(spawn.x, spawn.y))
			{
				bool overlap = false;
				for (auto& p : spawnpoints)
				{
					if (Vector2Distance(p, spawn) < 100.0f)
					{
						overlap = true;
						break;
					}
				}

				if (!overlap)
					valid = true;
			}
		}

		// 2. If center fails, try random angles
		if (!valid)
		{
			for (int attempts = 0; attempts < 200 && !valid; attempts++)
			{
				float offset = RandomFloat(-coneWidth * 0.5f, coneWidth * 0.5f);
				float angle = baseAngle + offset;

				spawn = {
					playerpos.x + r * cosf(angle),
					playerpos.y + r * sinf(angle)
				};

				if (!chunkmanager.IsWalkableAt(spawn.x, spawn.y))
					continue;

				bool overlap = false;
				for (auto& p : spawnpoints)
				{
					if (Vector2Distance(p, spawn) < 100.0f)
					{
						overlap = true;
						break;
					}
				}

				if (!overlap)
					valid = true;
			}
		}

		// 3. If still invalid, skip this patroler
		if (!valid)
			continue;

		// 4. Store spawnpoint
		spawnpoints.push_back(spawn);


		// 6. Generate path to player
		Vector2 target = { playerpos.x + 22.5f + RandomFloat(0, 10), playerpos.y + 36.0f + RandomFloat(0,10)}; // center of player
		vector<Vector2> path = FindPath(spawn, target, chunkmanager, 45, 72);

		// 7. Store path for drawing
		paths.push_back(path);
	}
	AssignPatrolers();
}

void LandPatrolHivemind::AssignPatrolers()
{
	// Clear existing minions
	for (LandPatroler* minion : minions)
	{
		delete minion;
		entitylayer.RemoveDrawCall(minion);
	}
	minions.clear();
	// Create new minions at spawnpoints with assigned paths
	for (size_t i = 0; i < spawnpoints.size(); i++)
	{
		LandPatroler* minion = new LandPatroler(static_cast<int>(i), spawnpoints[i], 100 * difficulty, this);
		minion->path = paths[i];
		minions.push_back(minion);
		entitylayer.AddDrawCall(minion, spawnpoints[i].y); // Add to entity layer for drawing
	}
}

void LandPatrolHivemind::Draw()
{
	// Draw spawn points
	for (Vector2 spawnpoint : spawnpoints)
	{
		DrawCircleV(spawnpoint, 3, RED);
	}

	// Draw paths with rainbow colors
	int index = 0;
	for (const vector<Vector2>& path : paths)
	{
		if (path.size() < 2)
		{
			index++;
			continue;
		}

		// Pick color for this path
		Color c = debugcolors[index % 6];

		// Draw line segments
		for (int i = 0; i < (int)path.size() - 1; i++)
		{
			DrawLineV(path[i], path[i + 1], c);
		}

		index++;
	}
}//nothing to draw, after generating hitpatrol, autmatically makes draw calls for all landpatrolers
void LandPatrolHivemind::PlayerNear() {};
void LandPatrolHivemind::PlayerFar() {};
void LandPatrolHivemind::Shutdown() {};
void LandPatrolHivemind::Startup() {};
#pragma endregion