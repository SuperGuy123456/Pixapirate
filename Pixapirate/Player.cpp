#include "Player.h"

Player::Player(EventManager& _playerposmanager, EventManager& _keyboardmanager, DrawLayer& _entitylayer, DrawLayer& _effectslayer, Camera2D& _camera) : playerposmanager(_playerposmanager), keyboardmanager(_keyboardmanager), entitylayer(_entitylayer), effectslayer(_effectslayer), cam(_camera)
{
	SpriteSplitter::Clothing("Art/Hats/hat_blackcaptain_clean.png",
		"Art/Head/light/head_light_midbeard_clean.png",
		"Art/Torso/light/torso_light_redrobes_clean.png",
		"Art/Arms/light/arms_light_simple_clean.png",
		"Art/Legs/light/legs_light_tanboots_used.png",
		hatframes, headframes, torsoframes, armsframes, legsframes); //Automatically sets all frames

	keyboardmanager.AddListener("player_listener", this, "HOLD_W");
	keyboardmanager.AddListener("player_listener_s", this, "HOLD_S");
	keyboardmanager.AddListener("player_listener_a", this, "HOLD_A");
	keyboardmanager.AddListener("player_listener_d", this, "HOLD_D");

	entitylayer.AddDrawCall(this, y);

	playerposmanager.BroadcastSpecialMessage("PLAYER_POS_UPDATE " + to_string(x) + " " + to_string(y));

	UpdateCamPos();
}

Player::~Player()
{
	for (AnimationFrames* bodypart : bodyframes)
	{
		for (Texture2D runtex : bodypart->run)
		{
			UnloadTexture(runtex);
		}

		for (Texture2D idletex : bodypart->idle)
		{
			UnloadTexture(idletex);
		}
	}
}

void Player::UpdateCamPos()
{
	cam.target = Vector2{ x, y };
}

void Player::Draw()
{
	for (AnimationFrames* bodypart : draworder)
	{
		Texture2D tex;

		if (state == AnimationState::IDLE)
		{
			if (frame == 3)
			{
				frame = 2; // Since run has 4 frames and idle only has 3
			}
			tex = bodypart->idle[frame];
		}
		else
		{
			tex = bodypart->run[frame];
		}

		// Source rectangle: full texture
		Rectangle src = { 0.0f, 0.0f, facingright * (float)tex.width, (float)tex.height };

		// Destination rectangle: position and size on screen
		Rectangle dest = { (float)x, (float)y, (float)tex.width, (float)tex.height };

		// Origin: pivot point (top-left here)
		Vector2 origin = { 0.0f, 0.0f };

		// Rotation: none
		float rotation = 0.0f;

		DrawTexturePro(tex, src, dest, origin, rotation, WHITE);
	}
}

void Player::Update()
{
	//Always update first then draw
	ApplyMovement();


	Animate();
	UpdateCamPos();
}

void Player::Animate()
{
	if (GetTime() - lasttime >= 0.2)
	{
		lasttime = GetTime();
		if (state == AnimationState::IDLE)
		{
			if (frame == 3) //if run prev played and ended in 4th frame, clamp to 0
			{
				frame = 0;
			}
			else if (frame == 2) //only has 3 frames
			{
				frame = 0;
			}
			else
			{
				frame++;
			}
		}
		else
		{
			if (frame == 3) //has 4 frames
			{
				frame = 0;
			}
			else
			{
				frame++;
			}
		}
	}
}

void Player::OnEvent(string& command)
{
	//cout << "Player received command: " << command << endl;
	if (command == "HOLD_W")
	{
		yvel -= 1;
		state = AnimationState::RUN;
		playerposmanager.BroadcastSpecialMessage("PLAYER_POS_UPDATE " + to_string(x) + " " + to_string(y));
	}
	else if (command == "HOLD_S")
	{
		yvel += 1;
		state = AnimationState::RUN;
		playerposmanager.BroadcastSpecialMessage("PLAYER_POS_UPDATE " + to_string(x) + " " + to_string(y));
	}
	else if (command == "HOLD_A")
	{
		xvel -= 1;
		state = AnimationState::RUN;
		playerposmanager.BroadcastSpecialMessage("PLAYER_POS_UPDATE " + to_string(x) + " " + to_string(y));
		facingright = -1;
	}
	else if (command == "HOLD_D")
	{
		xvel += 1;
		state = AnimationState::RUN;
		playerposmanager.BroadcastSpecialMessage("PLAYER_POS_UPDATE " + to_string(x) + " " + to_string(y));
		facingright = 1;
	}
	else
	{
		state = AnimationState::IDLE;
	}
}

void Player::ApplyMovement()
{
	if (xvel > 2)
	{
		xvel = 2;
	}
	else if (xvel < -2)
	{
		xvel = -2;
	}

	if (yvel > 2)
	{
		yvel = 2;
	}
	else if (yvel < -2)
	{
		yvel = -2;
	}

	x += xvel;
	y += yvel;

	if (xvel == 0 and yvel == 0)
	{
		state = AnimationState::IDLE;
	}
	else
	{
		entitylayer.ChangePriority(this, y);
	}

	if (xvel > 0)
	{
		xvel -= 0.5f;
	}
	else if (xvel < 0)
	{
		xvel += 0.5f;
	}

	if (yvel > 0)
	{
		yvel -= 0.5f;
	}
	else if (yvel < 0)
	{
		yvel += 0.5f;
	}
}