#pragma once
#include "BaseClasses/Base.h"
#include "Engine/Spritesplitter.h"
#include "Engine/EventManager.h"
#include "Engine/Allstructs.h"
#include <vector>
#include <iostream>
#include "Engine/DrawingPipeline.h"

enum AnimationState
{
	RUN,
	IDLE
};

using namespace std;

class Player : public HasCollider, public Load, public Listener
{
public:
	Player(EventManager& _playerposmanager, EventManager& _keyboardmanager, DrawLayer& _entitylayer, DrawLayer& _effectslayer, Camera2D& _camera); //Load ALL player texures
	~Player(); //Unload ALL player textures

	void Update(); // Ran before draw in mainloop
	void Draw() override; //Run by the DrawingPipline

	void OnEvent(string& command) override;
private:
	float x = 100;
	float y = 100;

	float xvel = 0;
	float yvel = 0;


	AnimationState state = AnimationState::IDLE;
	int frame = 0;
	double lasttime = GetTime();
	int facingright = 1;

	AnimationFrames hatframes;
	AnimationFrames headframes;
	AnimationFrames torsoframes;
	AnimationFrames armsframes;
	AnimationFrames legsframes;

	vector<AnimationFrames*> bodyframes = { &hatframes, &headframes, &torsoframes, &armsframes, &legsframes };
	vector<AnimationFrames*> draworder = { &torsoframes, &legsframes, &headframes, &armsframes, &hatframes };

	void Animate();
	void ApplyMovement();
	void UpdateCamPos();

	EventManager& playerposmanager;
	EventManager& keyboardmanager;

	DrawLayer& entitylayer;
	DrawLayer& effectslayer;

	Camera2D& cam;
};