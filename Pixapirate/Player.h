#pragma once
#include "BaseClasses/Base.h"
#include "Engine/Spritesplitter.h"
#include "Engine/Allstructs.h"
#include <vector>

enum AnimationState
{
	RUN,
	IDLE
};

using namespace std;

class Player : public HasCollider, public Load, public Listener
{
public:
	Player(); //Load ALL player texures
	~Player(); //Unload ALL player textures

	void Update(); // Ran before draw in mainloop
	void Draw(); //Run by the DrawingPipline
private:
	int x = 100;
	int y = 100;


	AnimationState state = AnimationState::IDLE;
	int frame = 0;

	AnimationFrames hatframes;
	AnimationFrames headframes;
	AnimationFrames torsoframes;
	AnimationFrames armsframes;
	AnimationFrames legsframes;

	vector<AnimationFrames*> bodyframes = { &hatframes, &headframes, &torsoframes, &armsframes, &legsframes };
};