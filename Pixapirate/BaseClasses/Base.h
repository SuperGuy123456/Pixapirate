#pragma once
#include <string>
#include <vector>

using namespace std;
// To store diff classes in same array
class HasCollider{};
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