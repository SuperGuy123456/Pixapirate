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
    // Split the message into words
    istringstream iss(message);
    vector<string> words;
    string word;

    while (iss >> word) {
        words.push_back(word);
    }

    if (words.empty()) return; // nothing to broadcast

    // First word is the command
    string command = words[0];

    // Remove the command from the vector so only params remain
    words.erase(words.begin());

    // Notify listeners
    for (auto const& x : speciallisteners)
    {
        if (x.second.command == command)
        {
            x.second.listener->OnSpecialEvent(command, words);
        }
    }
}