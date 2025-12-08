#pragma once

#include "../BaseClasses/Base.h"
#include <string>
#include "Allstructs.h"
#include <unordered_map>
#include <vector>
#include <sstream>

using namespace std;

class EventManager //Allows Broadcast messages that can trigger functions
{
public:
	//The string is the name of the load (allows for easy removal FAST) and the struct has the command and the listener object
	unordered_map<string, listenerload> listeners;
	unordered_map<string, listenerload> speciallisteners;
	
	void AddListener(string id, Listener* obj, string command);
	void AddSpecialListener(string id, Listener* obj, string command);
	void RemoveListener(string id);
	void RemoveSpecialListener(string id);

	void BroadcastMessage(string message);
	void BroadcastSpecialMessage(string message);
};

