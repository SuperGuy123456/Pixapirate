#pragma once

#include "raylib.h"
#include "../BaseClasses/Base.h"
#include <string>
#include <vector>

using namespace std;

struct AnimationFrames
{
	vector<Texture2D> run;
	vector<Texture2D> idle;
};

struct listenerload
{
	Listener listener;
	string command;
};