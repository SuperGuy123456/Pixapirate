#include "InputManager.h"

InputManager::InputManager(EventManager& _eventmanager) : eventmanager(_eventmanager)
{
	eventmanager = _eventmanager;
}
InputManager::~InputManager() {};

void InputManager::GetInput() //Call every frame to check for input and broadcast events
{
	//Check hold keys
	for (int i = 0; i < 5; i++)
	{
		if (IsKeyDown(holdkeys[i]))
		{
			eventmanager.BroadcastMessage("HOLD_" + holdkeysreadable[i]);
			//cout << "HOLD_" + holdkeysreadable[i] << endl;
		}
	}
	//Check press keys
	for (int i = 0; i < 6; i++)
	{
		if (IsKeyPressed(presskeys[i]))
		{
			eventmanager.BroadcastMessage("PRESS_" + presskeysreadable[i]);
			//cout << "PRESS_" + presskeysreadable[i] << endl;
		}
	}
}
