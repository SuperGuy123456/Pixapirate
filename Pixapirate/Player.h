#pragma once
#include "BaseClasses/Base.h"


class Player : public HasCollider, public Load
{
public:
	Player(); //Load ALL player texures
	~Player(); //Unload ALL player textures

	void Update(); // Ran before draw in mainloop
	void Draw(); //Run by the DrawingPipline
};

