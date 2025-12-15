#include "AI.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

struct Node {
	int x, y;
	float gCost, hCost;
	Node* parent;
	float fCost() const { return gCost + hCost; }
};

bool IsAreaWalkable(int wx, int wy, int w, int h, ChunkManager& cm)
{
	int leftTile = wx / 48;
	int rightTile = (wx + w - 1) / 48;
	int topTile = wy / 48;
	int bottomTile = (wy + h - 1) / 48;

	for (int ty = topTile; ty <= bottomTile; ty++)
	{
		for (int tx = leftTile; tx <= rightTile; tx++)
		{
			if (!cm.IsWalkableAt(tx * 48, ty * 48))
				return false;
		}
	}

	return true;
}

std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, ChunkManager& chunkManager, int objWidth, int objHeight)
{
	// Convert to tile coordinates
	int startX = startPos.x / 48;
	int startY = startPos.y / 48;
	int endX = endPos.x / 48;
	int endY = endPos.y / 48;

	// ✅ Reject invalid target tile
	if (!IsAreaWalkable(endX * 48, endY * 48, objWidth, objHeight, chunkManager))
	{
		std::cout << "Pathfind aborted: target not walkable\n";
		return {};
	}

	struct Node {
		int x, y;
		float gCost, hCost;
		Node* parent;
		float fCost() const { return gCost + hCost; }
	};

	auto cmp = [](Node* a, Node* b) { return a->fCost() > b->fCost(); };
	std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> openSet(cmp);

	std::unordered_map<long long, Node*> allNodes;

	auto getNode = [&](int x, int y) -> Node*
		{
			long long key = ((long long)x << 32) | (unsigned long long)y;
			if (allNodes.count(key)) return allNodes[key];

			Node* n = new Node{ x, y, INFINITY, 0, nullptr };
			allNodes[key] = n;
			return n;
		};

	Node* start = getNode(startX, startY);
	Node* goal = getNode(endX, endY);

	start->gCost = 0;
	start->hCost = sqrtf((startX - endX) * (startX - endX) + (startY - endY) * (startY - endY));
	openSet.push(start);

	std::unordered_set<long long> closed;

	// ✅ 8-direction movement
	const int dirs[8][2] = {
		{ 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },   // cardinal
		{ 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 }  // diagonals
	};

	while (!openSet.empty())
	{
		Node* current = openSet.top();
		openSet.pop();

		if (current == goal)
			break;

		long long ckey = ((long long)current->x << 32) | (unsigned long long)current->y;
		closed.insert(ckey);

		for (auto& d : dirs)
		{
			int nx = current->x + d[0];
			int ny = current->y + d[1];

			long long nkey = ((long long)nx << 32) | (unsigned long long)ny;
			if (closed.count(nkey)) continue;

			bool diagonal = (d[0] != 0 && d[1] != 0);

			// ✅ Prevent diagonal corner-cutting
			if (diagonal)
			{
				if (!IsAreaWalkable((current->x + d[0]) * 48, current->y * 48, objWidth, objHeight, chunkManager))
					continue;
				if (!IsAreaWalkable(current->x * 48, (current->y + d[1]) * 48, objWidth, objHeight, chunkManager))
					continue;
			}

			// ✅ Check walkability of the neighbor tile
			if (!IsAreaWalkable(nx * 48, ny * 48, objWidth, objHeight, chunkManager))
				continue;

			Node* neighbor = getNode(nx, ny);

			float stepCost = diagonal ? 1.41421356f : 1.0f;
			float newCost = current->gCost + stepCost;

			if (newCost < neighbor->gCost)
			{
				neighbor->gCost = newCost;
				neighbor->hCost = sqrtf((nx - endX) * (nx - endX) + (ny - endY) * (ny - endY));
				neighbor->parent = current;
				openSet.push(neighbor);
			}
		}
	}

	// ✅ If unreachable, return empty
	if (goal->parent == nullptr && goal != start)
	{
		std::cout << "No path found\n";
		return {};
	}

	// ✅ Reconstruct path
	std::vector<Vector2> path;
	Node* cur = goal;

	while (cur != nullptr)
	{
		path.push_back(Vector2{ cur->x * 48.0f, cur->y * 48.0f });
		cur = cur->parent;
	}

	std::reverse(path.begin(), path.end());
	return path;
}
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

	if (!patrol) {currentpath = FindPath(position, patrolpoints[0], chunkmanager, 45, 72); }
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

	if (!patrol) { currentpath = FindPath(position, patrolpoints[0], chunkmanager, 45, 72); }
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
	//movement or other stuff go here
	//Always update first then draw
	UpdateFrame();

	//if patrolling, should always have a path to follow
	if (patrol)
	{
		if (!currentpath.empty())
		{
			Vector2 next = currentpath[0];

			// Move 1 pixel toward the next tile
			Vector2 dir = Vector2Normalize(Vector2Subtract(next, position));


			position.x += dir.x;
			position.y += dir.y;

			if (dir.x > 0)
			{
				facingright = 1;
			}
			else if (dir.x < 0)
			{
				facingright = -1;
			}

			// If close enough, pop the waypoint
			if (fabs(position.x - next.x) < 2 && fabs(position.y - next.y) < 2)
			{
				position = next; // ✅ Snap to tile center
				currentpath.erase(currentpath.begin());
			}
		}
		else
		{
			currentpatrolpoint = (currentpatrolpoint + 1) % patrolpoints.size();
			currentpath = FindPath(position, patrolpoints[currentpatrolpoint], chunkmanager, 45, 72);
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
	for (const Vector2& point : currentpath)
	{
		DrawCircleV(point, 5.0f, RED);
	}
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
}

void SimpleNPC::Startup()
{
	// TEMP: do nothing
	SpriteSplitter::Clothing("Art/Hats/hat_placeholder_clean.png",
		"Art/Head/light/head_light_midbeard_clean.png",
		"Art/Torso/light/torso_light_sailor_clean.png",
		"Art/Arms/light/arms_light_simple_clean.png",
		"Art/Legs/light/legs_light_tanboots_used.png",
		hatframes, headframes, torsoframes, armsframes, legsframes); //Automatically sets all frames
}

