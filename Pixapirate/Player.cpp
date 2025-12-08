#include "Player.h"

Player::Player(EventManager& _playerposmanager, EventManager& _keyboardmanager) : playerposmanager(_playerposmanager), keyboardmanager(_keyboardmanager)
{
	SpriteSplitter::Clothing("Art/Hats/hat_blackcaptain_clean.png",
		"Art/Head/light/head_light_midbeard_clean.png",
		"Art/Torso/light/torso_light_redrobes_clean.png",
		"Art/Arms/light/arms_light_simple_clean.png",
		"Art/Legs/light/legs_light_tanboots_used.png",
		hatframes, headframes, torsoframes, armsframes, legsframes); //Automatically sets all frames

	playerposmanager = _playerposmanager;
	keyboardmanager = _keyboardmanager;

	keyboardmanager.AddListener("player_listener", this, "HOLD_W");
	keyboardmanager.AddListener("player_listener_s", this, "HOLD_S");
	keyboardmanager.AddListener("player_listener_a", this, "HOLD_A");
	keyboardmanager.AddListener("player_listener_d", this, "HOLD_D");
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

void Player::Draw()
{
	for (AnimationFrames* bodypart : draworder)
	{
		if (state == AnimationState::IDLE)
		{
			if (frame == 3)
			{
				frame = 2; //Since run has 4 frames and idle only has 3
			}
			DrawTexture(bodypart->idle[frame], x, y, Color{ 255,255,255, 255});
		}
		else
		{
			DrawTexture(bodypart->run[frame], x, y, Color{ 255,255,255, 255});
		}
	}
}

void Player::Update()
{
	//Always update first then draw
	ApplyMovement();


	Animate();
}

void Player::Animate()
{
	if (GetTime() - lasttime >= 0.2)
	{
		lasttime = GetTime();
		if (state == AnimationState::IDLE)
		{
			if (frame == 2) //only has 3 frames
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
	}
	else if (command == "HOLD_S")
	{
		yvel += 1;
		state = AnimationState::RUN;
	}
	else if (command == "HOLD_A")
	{
		xvel -= 1;
		state = AnimationState::RUN;
	}
	else if (command == "HOLD_D")
	{
		xvel += 1;
		state = AnimationState::RUN;
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