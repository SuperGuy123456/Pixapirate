#include "Pathfinding.h"

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

bool HasLineOfSight(Vector2 a, Vector2 b, int objWidth, int objHeight, ChunkManager& chunkManager)
{
	// Sample along the line every few pixels
	const float step = 8.0f;

	Vector2 dir = Vector2Normalize(Vector2Subtract(b, a));
	float dist = Vector2Distance(a, b);

	for (float t = 0; t < dist; t += step)
	{
		Vector2 p = Vector2Add(a, Vector2Scale(dir, t));
		if (!IsAreaWalkable(p.x, p.y, objWidth, objHeight, chunkManager))
			return false;
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

    // Reject invalid target tile
    if (!IsAreaWalkable(endX * 48, endY * 48, objWidth, objHeight, chunkManager))
    {
        std::cout << "Pathfind aborted: target not walkable\n";
        return {};
    }

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

    const int dirs[8][2] = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
        { 1, 1 }, { 1,-1 }, { -1,1 }, { -1,-1 }
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

            // Prevent diagonal corner cutting
            if (diagonal)
            {
                if (!IsAreaWalkable((current->x + d[0]) * 48, current->y * 48, objWidth, objHeight, chunkManager))
                    continue;
                if (!IsAreaWalkable(current->x * 48, (current->y + d[1]) * 48, objWidth, objHeight, chunkManager))
                    continue;
            }

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

    if (goal->parent == nullptr && goal != start)
    {
        std::cout << "No path found\n";
        return {};
    }

    // Reconstruct path (tile centers)
    std::vector<Vector2> path;
    Node* cur = goal;

    while (cur != nullptr)
    {
        path.push_back(Vector2{
            cur->x * 48.0f + 24.0f,
            cur->y * 48.0f + 24.0f
            });
        cur = cur->parent;
    }

    std::reverse(path.begin(), path.end());

    // Smooth the path
    std::vector<Vector2> smooth;
    if (!path.empty())
    {
        // Use exact pixel start
        smooth.push_back(startPos);

        for (int i = 1; i < path.size(); i++)
        {
            if (!HasLineOfSight(smooth.back(), path[i], objWidth, objHeight, chunkManager))
            {
                smooth.push_back(path[i - 1]);
            }
        }

        // Exact pixel end
        smooth.push_back(endPos);
    }

    return smooth;
}
std::vector<Vector2> BuildPatrolPath(
	const std::vector<Vector2>& patrolPoints,
	ChunkManager& chunkManager,
	int objWidth,
	int objHeight)
{
	std::vector<Vector2> fullPath;

	for (int i = 0; i < patrolPoints.size(); i++)
	{
		Vector2 start = patrolPoints[i];
		Vector2 end = patrolPoints[(i + 1) % patrolPoints.size()]; // loop

		std::vector<Vector2> segment = FindPath(start, end, chunkManager, objWidth, objHeight);

		if (segment.empty())
			continue;

		// Avoid duplicating the first point of each segment
		if (!fullPath.empty())
			segment.erase(segment.begin());

		fullPath.insert(fullPath.end(), segment.begin(), segment.end());
	}

	return fullPath;
}