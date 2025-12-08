#pragma once
#include "raylib.h"
#include "EventManager.h"
#include <string>
#include <iostream>

class InputManager //Basically enhanced input actions but c++ style and just uses event mnaager to broadcast button presses
{
public:
	KeyboardKey holdkeys[5] = { KEY_W, KEY_A, KEY_S, KEY_D, KEY_SPACE };
	string holdkeysreadable[5] = { "W", "A", "S", "D", "Space" };

	KeyboardKey presskeys[6] = { KEY_E, KEY_Q, KEY_O, KEY_ESCAPE, KEY_SEMICOLON, KEY_LEFT_BRACKET };
	string presskeysreadable[6] = { "E", "Q", "O", "Escape", ";", "[" };

	InputManager(EventManager& _eventmanager);
	~InputManager();

	void GetInput(); //Call every frame to check for input and broadcast events

private:
	EventManager& eventmanager;
};

