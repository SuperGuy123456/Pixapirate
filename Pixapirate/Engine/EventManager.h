#pragma once

#include "../BaseClasses/Base.h"
#include "../Engine/Allstructs.h"
#include <vector>
#include <string>

using namespace std;

class EventManager //Allows Broadcast messages that can trigger functions
{
public:
	vector<listenerload> listeners;
	vector<listenerload> speciallisteners; //Basically for functions that need params (sent as an array of strings) and each param is splitted by whitespace

	
};

