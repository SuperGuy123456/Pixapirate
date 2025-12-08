// Pixapirate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "raylib.h"
#include <iostream>
#include "Player.h"
#include "Engine/EventManager.h"
#include "Engine/InputMAnager.h"

int main()
{
    InitWindow(1920, 1080, "Pixapirate");
    SetWindowPosition(0, 30); //So the player can see the top bar

    SetTargetFPS(60);

    EventManager playerposmanager;
    EventManager keyboardmanager;

	InputManager inputmanager(keyboardmanager);

    Player player(playerposmanager, keyboardmanager);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(Color{ 0,0,0 });

		inputmanager.GetInput();

        //----------------------------------UPDATES--------------------
        player.Update();

        //-------------------------------DRAWCALLS---------------------
        player.Draw();
        
        DrawFPS(0 ,20);
        EndDrawing();
    }

    CloseWindow();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
