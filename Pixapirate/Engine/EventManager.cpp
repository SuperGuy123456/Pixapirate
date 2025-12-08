#include "EventManager.h"
void EventManager::AddListener(string id, Listener* obj, string command)
{
	listenerload l; //temporary

	l.command = command;
	l.listener = obj;

	listeners[id] = l;
}
void EventManager::AddSpecialListener(string id, Listener* obj, string command)
{
	listenerload l; //temporary

	l.command = command;
	l.listener = obj;

	speciallisteners[id] = l;
}


void EventManager::RemoveListener(string id)
{
	//Find item using id and remove it
	listeners.erase(id);
}
void EventManager::RemoveSpecialListener(string id)
{
	//same but for special listeners
	speciallisteners.erase(id);
}


void EventManager::BroadcastMessage(string message)
{
	for (auto const& x : listeners)
	{
		if (x.second.command == message)
		{
			x.second.listener->OnEvent(message);
		}
	}
}
void EventManager::BroadcastSpecialMessage(string message)
{
	//the boradcast is basically the keyword and a bunch of params separated by spaces
	//eg: command could be damage so a special broadcast would be damage 20 or something like that

	//first split string by space and discard first part
	istringstream iss(message);

	vector<std::string> words;
	string word;

	while (iss >> word) {
		words.push_back(word);
	}

	string command = words[0]; //first word is the command

	words.erase(words.begin()); //remove first word so only params remain

	for (auto const& x : speciallisteners)
	{
		if (x.second.command == command)
		{
			x.second.listener->OnSpecialEvent(message, words);
		}
	}

}