// Pixapirate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "raylib.h"
#include <iostream>
#include "Player.h"
#include "Engine/EventManager.h"
#include "Engine/InputManager.h"
#include "Engine/DrawingPipeline.h"
#include "Engine/Chunks.h"

int main()
{
    InitWindow(1920, 1080, "Pixapirate");
    SetWindowPosition(0, 30); //So the player can see the top bar

    SetTargetFPS(60);

	Camera2D camera = { 0 };
    camera.target = Vector2{ 100, 100 };   // follow player
    camera.offset = Vector2{ 1920 / 2.0f, 1080 / 2.0f }; // keep player centered
    camera.rotation = 0.0f;
    camera.zoom = 2.0f;


	DrawingPipeline drawingpipeline;

    DrawLayer uilayer;
    DrawLayer effectslayer;
    DrawLayer entitylayer;
    DrawLayer bglayer;

	drawingpipeline.AddLayer(&uilayer, "UI");
	drawingpipeline.AddLayer(&effectslayer, "Effects");
	drawingpipeline.AddLayer(&entitylayer, "Entities");
	drawingpipeline.AddLayer(&bglayer, "Background");

    EventManager playerposmanager;
    EventManager keyboardmanager;

	InputManager inputmanager(keyboardmanager);

    Player player(playerposmanager, keyboardmanager, entitylayer, effectslayer, camera);

	ChunkManager chunkmanager(playerposmanager, bglayer);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(Color{ 0, 79,130 });

		inputmanager.GetInput();
        if (IsKeyPressed(KEY_F1)) camera.zoom = 2.0f;   // normal play view
        if (IsKeyPressed(KEY_F2)) camera.zoom = 1.0f;   // slight zoom out
        if (IsKeyPressed(KEY_F3)) camera.zoom = 0.5f;   // debug overview
        if (IsKeyPressed(KEY_F4)) camera.zoom = 0.25f;  // extreme zoom-out

        //----------------------------------UPDATES--------------------
        player.Update();
        chunkmanager.Update();

        BeginMode2D(camera);

        //-------------------------------DRAWCALLS---------------------
        drawingpipeline.DrawAll();

		EndMode2D();
        
        DrawFPS(0 ,20);
        EndDrawing();
    }

    CloseWindow();
}
