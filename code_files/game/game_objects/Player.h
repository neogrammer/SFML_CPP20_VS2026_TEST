#pragma once
#include <game/game_objects/AnimObj.h>
#include <cstddef>
#include <vector>

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
	struct DamagePop
	{
		sf::Vector2f position{};
		sf::Vector2f velocity{};
		int amount{ 0 };
		float timer{ 0.0f };
		float lifetime{ 0.75f };
	};

	Player();
	~Player() override;

	const GObj& operator()();
	void resetCombatState();
	void updateCombatTimers(float dt);
	bool tryStartShot(std::size_t activeShotCount, std::size_t maxShots);
	sf::Vector2f getShotSpawnPosition(float shotRadius);
	PlayerJumpAnimPhase updateJumpAnimationPhase(bool wasGrounded, float dt, float jumpVelocity, float gravityPerFrame);
	bool takeDamage(int damage);
	void heal(int amount);
	bool shouldBlinkOff() const;
	bool isHitFlashActive() const;
	const std::vector<DamagePop>& getDamagePops() const;

	bool movingRight{ false };
	bool movingLeft{ false };

	PlayerJumpAnimPhase jumpAnimPhase{ PlayerJumpAnimPhase::Grounded };
	float jumpStartY{ 0.0f };
	float jumpPeakY{ 0.0f };
	float jumpExpectedHeight{ 1.0f };
	int jumpAnimFramesInAir{ 0 };

	int maxHealth{ 25 };
	int health{ 25 };

private:
	void spawnHealthDamagePop(int damage);

	std::vector<DamagePop> damagePops{};
	float shotCooldown{ 0.0f };
	float shootPoseTimer{ 0.0f };
	float invincibleTimer{ 0.0f };
	float hitFlashTimer{ 0.0f };
};
