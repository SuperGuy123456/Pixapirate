// Pixapirate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//Character is 15 pixels wide and 24 pixels tall or 45 x 72 pixels in game (3x scale)

#include "raylib.h"
#include <iostream>
#include "Player.h"
#include "Engine/EventManager.h"
#include "Engine/InputManager.h"
#include "Engine/DrawingPipeline.h"
#include "Engine/Chunks.h"
#include "AI.h"

int main()
{
    InitWindow(1920, 1080, "Pixapirate");
    SetWindowPosition(0, 30); //So the player can see the top bar

    SetTargetFPS(300);

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

	ChunkManager chunkmanager(playerposmanager, bglayer, entitylayer);

    /*SimpleNPC* testNPC = new SimpleNPC(
        "test_npc",
        Vector2{ 367, 246 },
        100,
        playerposmanager,
        chunkmanager,
        vector<Vector2>{
        Vector2{ 367.0f, 246.0f },
            Vector2{ 933.0f, 503.0f },
            Vector2{ 965.0f, 861.0f },
            Vector2{ 181.0f, 722.0f }
    },
        false
    );*/

    LandPatrolHivemind* testhivemind = new LandPatrolHivemind(
        1,
        8,
        false,
        entitylayer,
        playerposmanager,
        chunkmanager
    );

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(Color{ 0, 79,130 });

		inputmanager.GetInput();
        if (IsKeyPressed(KEY_F1)) camera.zoom = 2.0f;   // normal play view
        if (IsKeyPressed(KEY_F2)) camera.zoom = 1.0f;   // slight zoom out
        if (IsKeyPressed(KEY_F3)) camera.zoom = 0.5f;   // debug overview
        if (IsKeyPressed(KEY_F4)) camera.zoom = 0.25f;  // extreme zoom-out

		chunkmanager.camera = camera;

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
