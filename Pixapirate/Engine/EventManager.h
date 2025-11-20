#pragma once

#include "../BaseClasses/Base.h"
#include "../Engine/Allstructs.h"
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class EventManager //Allows Broadcast messages that can trigger functions
{
public:
	//The string is the name of the load (allows for easy removal FAST)
	unordered_map<string, listenerload> listeners; 
	unordered_map<string, listenerload> speciallisteners;
	
	void AddListener(string id, Listener obj, string command);
	void AddSpecialListener(string id, Listener obj, string command);
	void RemoveListener(string id);
	void RemoveSpecialListener(string is);

	void BroadcastMessage(string message);
	void BroadcastSpecialMessage(string message);
};

