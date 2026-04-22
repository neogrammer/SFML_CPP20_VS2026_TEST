#pragma once
#include <game/game_objects/AnimObj.h>

enum class PlayerJumpAnimPhase
{
	Grounded,
	LiftOff,
	Rising,
	JumpPeak,
	Falling,
	Landing
};

class Player : public AnimObj
{
public:
	Player();
	~Player() override;

	const GObj& operator()();

	bool movingRight{ false };
	bool movingLeft{ false };

	PlayerJumpAnimPhase jumpAnimPhase{ PlayerJumpAnimPhase::Grounded };
	float jumpStartY{ 0.0f };
	float jumpPeakY{ 0.0f };
	float jumpExpectedHeight{ 1.0f };
	int jumpAnimFramesInAir{ 0 };

	int maxHealth{ 25 };
	int health{ 25 };
};
