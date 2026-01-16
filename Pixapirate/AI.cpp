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
	this->id = id;

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
	}
}

void LandPatroler::Draw()
{
	for (AnimationFrames* bodypart : draworder)
	{
		Texture2D tex;
		tex = bodypart->run[cframe];
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

	/*for (Vector2 point : path)
	{
		DrawCircleV(point, 5.0f, GREEN);
	}*/
}

void LandPatroler::Update()
{
	if (!path.empty())
	{
		Vector2 next = path[point];

		// 1. Path direction (macro movement)
		Vector2 dir = Vector2Normalize(Vector2Subtract(next, position));

		// 2. Separation steering (micro movement)
		Vector2 separation = { 0, 0 };

		for (LandPatroler* other : hivemindpointer->minions)
		{
			if (other == this) continue;

			float d = Vector2Distance(position, other->position);

			// Personal space radius
			const float radius = 60.0f;

			if (d < radius && d > 0.001f)
			{
				Vector2 push = Vector2Normalize(Vector2Subtract(position, other->position));
				float strength = (radius - d) / radius; // stronger when closer

				separation.x += push.x * strength;
				separation.y += push.y * strength;
			}
		}

		// 3. Blend path direction + separation
		Vector2 finalDir = Vector2Add(dir, separation);

		// Normalize final direction (avoid speed spikes)
		if (Vector2Length(finalDir) > 0.001f)
			finalDir = Vector2Normalize(finalDir);
		else
			finalDir = dir; // fallback

		// 4. Move
		const float speed = 2.5f;
		position.x += finalDir.x * speed;
		position.y += finalDir.y * speed;

		// 5. Facing direction
		if (finalDir.x > 0)      facingright = 1;
		else if (finalDir.x < 0) facingright = -1;

		// 6. Check waypoint arrival (AFTER steering)
		if (fabs(position.x - next.x) < 2 && fabs(position.y - next.y) < 2)
		{
			position = next; // Snap cleanly

			if (point + 1 >= path.size())
			{
				path.clear();
				return;
			}

			point++;
		}
	}
	else
	{
		// Tell hivemind that point has been reached
		hivemindpointer->LANDPATROL_ReachedTarget(this);
	}

	UpdateFrame();
}

void LandPatroler::HIVEMIND_StartShooting()//no shooting mechanics yet TODO
{
	shoot = true;
}

void LandPatroler::HIVEMIND_ChangeState(ChaseStage newstage)
{
	task = newstage;
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
		taskboard = {
		{ ROLE_REAR, nullptr },
		{ ROLE_SIDE_LEFT, nullptr },
		{ ROLE_SIDE_RIGHT, nullptr },
		{ ROLE_SUPPORT_BOTTOM_LEFT, nullptr },
		{ ROLE_SUPPORT_BOTTOM_RIGHT, nullptr},
		{ ROLE_SUPPORT_TOP_LEFT, nullptr },
		{ ROLE_SUPPORT_TOP_RIGHT, nullptr },
		{ ROLE_CUTOFF, nullptr }
			};
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

Vector2 LandPatrolHivemind::ComputeStartFlankPoint(
	float playerSpeed,
	Vector2 enemyPos,
	float enemySpeed)
{
	// Normalize forward vector
	Vector2 fwd = Vector2Normalize(playerforwardvector);

	// Start with a simple guess: player's current position
	Vector2 X = playerpos;

	const float behindDist = 100.0f;

	// Iterate to converge on the correct intercept point
	for (int i = 0; i < 5; i++)
	{
		// Time for enemy to reach X
		float distEnemy = Vector2Distance(enemyPos, X);
		float t = distEnemy / enemySpeed;

		// Predict where the player will be at time t
		Vector2 predictedPlayer = {
			playerpos.x + fwd.x * playerSpeed * t,
			playerpos.y + fwd.y * playerSpeed * t
		};

		// New X = 100px behind predicted player position
		X = {
			predictedPlayer.x - fwd.x * behindDist,
			predictedPlayer.y - fwd.y * behindDist
		};
	}

	return X;
}

void LandPatrolHivemind::AssignFlankRoles(LandPatroler* minion)
{
	// If this minion already has a slot, reuse it
	for (TaskSlot& slot : taskboard)
	{
		if (slot.assigned == minion)
		{
			CalculateFlankPoint(minion, &slot);
			return;
		}
	}

	// Otherwise assign a new slot
	for (TaskSlot& slot : taskboard)
	{
		if (slot.assigned == nullptr)
		{
			slot.assigned = minion;
			CalculateFlankPoint(minion, &slot);
			return;
		}
	}

	// If we reach here, something is wrong (more minions than roles)
	// You can add a fallback if you want
}

void LandPatrolHivemind::CalculateFlankPoint(LandPatroler* minion, TaskSlot* slotgiven)
{
	// Basic vectors
	Vector2 forward = Vector2Normalize(playerforwardvector);
	Vector2 right = Vector2Normalize(Vector2{ forward.y, -forward.x });

	float npcSpeed = 2.5f;
	float playerSpeed = 2.0f;

	// Estimate time to reach player
	float dist = Vector2Distance(minion->position, playerpos);
	float t = dist / npcSpeed;

	// Predict future player position
	Vector2 futurePlayer = {
		playerpos.x + forward.x * playerSpeed * t,
		playerpos.y + forward.y * playerSpeed * t
	};


	float forwardOffset = 0.0f;
	float rightOffset = 0.0f;

	const float baseDist = 130.0f;

	switch (slotgiven->role)
	{
	case ROLE_REAR:
		forwardOffset = -1.6f;   // slightly closer than -2.0
		rightOffset = 0.0f;
		break;

	case ROLE_SIDE_LEFT:
		forwardOffset = -0.2f;   // slightly behind player
		rightOffset = -1.2f;     // was -2.0 (too wide)
		break;

	case ROLE_SIDE_RIGHT:
		forwardOffset = -0.2f;
		rightOffset = +1.2f;     // was +2.0
		break;

	case ROLE_SUPPORT_TOP_LEFT:
		forwardOffset = +0.4f;   // was +1.0 (too far ahead)
		rightOffset = -0.9f;     // was -1.0 (slightly tighten)
		break;

	case ROLE_SUPPORT_TOP_RIGHT:
		forwardOffset = +0.4f;
		rightOffset = +0.9f;
		break;

	case ROLE_SUPPORT_BOTTOM_LEFT:
		forwardOffset = -0.8f;   // slightly closer
		rightOffset = -1.2f;     // match SIDE_LEFT width
		break;

	case ROLE_SUPPORT_BOTTOM_RIGHT:
		forwardOffset = -0.8f;
		rightOffset = +1.2f;
		break;

	case ROLE_CUTOFF:
		forwardOffset = +1.2f;   // was +3.0 (WAY too far)
		rightOffset = 0.0f;
		break;
	}

	// Compute flank point relative to future player position
	Vector2 flankPoint = Vector2Add(
		futurePlayer,
		Vector2Add(
			Vector2Scale(forward, forwardOffset * baseDist),
			Vector2Scale(right, rightOffset * baseDist)
		)
	);

	minion->path = FindPath(minion->position, flankPoint, chunkmanager, 45, 72);
	minion->point = 0;
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

	if (lastforward.x != playerforwardvector.x || lastforward.y != playerforwardvector.y)
	{
		lastforward = playerforwardvector;

		for (LandPatroler* minion : minions)
		{
			switch (minion->task)
			{
			case ChaseStage::APPROACH:
			{
				// recalc path to player
				minion->path.clear();
				minion->path = FindPath(minion->position, Vector2{ playerpos.x + 22.5f, playerpos.y + 36.0f}, chunkmanager, 45, 72);
				minion->point = 0;
				break;
			}

			case ChaseStage::BEGINFLANK:
			{
				// recalc flank start point
				flankstarts[minion->id] = ComputeStartFlankPoint(2.0f, minion->position, 2.25);
				minion->path.clear();
				minion->path = FindPath(minion->position, flankstarts[minion->id], chunkmanager, 45, 72);
				minion->point = 0;
				break;
			}

			case ChaseStage::FLANKING:
			{
				for (TaskSlot& slot : taskboard)
				{
					if (slot.assigned == minion)
					{
						CalculateFlankPoint(minion, &slot);
						break;
					}
				}
			}
			}
		}
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
float LandPatrolHivemind::RandomFloat(float min, float max)
{
	return min + (float)GetRandomValue(0, 1000000) / 1000000.0f * (max - min);
}

void LandPatrolHivemind::DebugSpawnHitPatrols()
{
	spawnpoints.clear();
	paths.clear();

	// Normalize forward vector
	Vector2 forward = playerforwardvector;
	if (forward.x != 0 || forward.y != 0)
		forward = Vector2Normalize(forward);
	else
		forward = { 1, 0 };

	float playerAngle = atan2f(forward.y, forward.x);
	float baseAngle = playerAngle + PI; // directly behind player

	float coneWidth = PI / 2.0f; // 90 degrees
	float r = 600.0f;

	// ---------------------------------------------------------
	// STEP 1: Find ONE valid spawn point in the cone
	// ---------------------------------------------------------
	Vector2 spawn = { 0, 0 };
	bool found = false;

	// Try center first
	{
		float angle = baseAngle;
		spawn = {
			playerpos.x + r * cosf(angle),
			playerpos.y + r * sinf(angle)
		};

		if (chunkmanager.IsWalkableAt(spawn.x, spawn.y))
			found = true;
	}

	// If center fails, try random angles
	if (!found)
	{
		for (int attempts = 0; attempts < 300 && !found; attempts++)
		{
			float offset = RandomFloat(-coneWidth * 0.5f, coneWidth * 0.5f);
			float angle = baseAngle + offset;

			spawn = {
				playerpos.x + r * cosf(angle),
				playerpos.y + r * sinf(angle)
			};

			if (chunkmanager.IsWalkableAt(spawn.x, spawn.y))
				found = true;
		}
	}

	// If still no valid spawn, abort
	if (!found)
		return;

	// Store the single spawn point
	spawnpoints.push_back(spawn);

	// ---------------------------------------------------------
	// STEP 2: Generate paths for each NPC from the SAME spawn
	// ---------------------------------------------------------
	Vector2 target = { playerpos.x + 22.5f, playerpos.y + 36.0f };

	for (int i = 0; i < size; i++)
	{
		vector<Vector2> path = FindPath(spawn, target, chunkmanager, 45, 72);
		paths.push_back(path);
	}

	// ---------------------------------------------------------
	// STEP 3: Assign patrolers
	// ---------------------------------------------------------
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
	flankstarts.clear();
	// Create new minions at spawnpoints with assigned paths
	for (size_t i = 0; i < size; i++)
	{
		LandPatroler* minion = new LandPatroler(static_cast<int>(i), spawnpoints[0], 100 * difficulty, this);
		minion->path = paths[i];
		minions.push_back(minion);
		flankstarts.push_back(ComputeStartFlankPoint(2.0f, spawnpoints[0], 2.25f));
		entitylayer.AddDrawCall(minion, spawnpoints[0].y); // Add to entity layer for drawing
	}
}

void LandPatrolHivemind::Draw()
{
	// Draw spawn points
	/*for (Vector2 spawnpoint : spawnpoints)
	{
		DrawCircleV(spawnpoint, 3, RED);
	}
	for (Vector2 flankstart : flankstarts)
	{
		DrawCircleV(flankstart, 3, BLUE);
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
	}*/
}//nothing to draw, after generating hitpatrol, autmatically makes draw calls for all landpatrolers

void LandPatrolHivemind::LANDPATROL_ReachedTarget(LandPatroler* minion)
{
	switch (minion->task)
	{
	case ChaseStage::APPROACH:
		//give beginflankpoint TODO
		minion->HIVEMIND_ChangeState(ChaseStage::BEGINFLANK);
		minion->point = 0;
		minion->path = FindPath(minion->position, flankstarts[minion->id], chunkmanager, 45, 72); //assign flankstart point
		break;
	case ChaseStage::BEGINFLANK:
		//give flankpoint/Role TODO
		minion->HIVEMIND_ChangeState(ChaseStage::FLANKING);
		minion->HIVEMIND_StartShooting();
		AssignFlankRoles(minion);
		break;
	case ChaseStage::FLANKING:
		//assign point that is even further ahead TODO
		for (TaskSlot& slot : taskboard)
		{
			if (slot.assigned == minion)
			{
				CalculateFlankPoint(minion, &slot);
				break;
			}
		}
	}
}

void LandPatrolHivemind::PlayerNear() {};
void LandPatrolHivemind::PlayerFar() {};
void LandPatrolHivemind::Shutdown() {};
void LandPatrolHivemind::Startup() {};
#pragma endregion