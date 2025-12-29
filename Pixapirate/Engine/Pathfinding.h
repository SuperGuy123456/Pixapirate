#pragma once
#include "../BaseClasses/Base.h"
#include <vector>
#include "Chunks.h"
#include "raylib.h"
#include <iostream>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "raymath.h"
#include "Allstructs.h"

bool IsAreaWalkable(int wx, int wy, int w, int h, ChunkManager& cm);
bool HasLineOfSight(Vector2 a, Vector2 b, int objWidth, int objHeight, ChunkManager& chunkManager);
std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, ChunkManager& chunkManager, int objWidth, int objHeight);
std::vector<Vector2> BuildPatrolPath(const std::vector<Vector2>& patrolPoints, ChunkManager& chunkManager, int objWidth, int objHeight);
