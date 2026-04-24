#pragma once
#include <game/game_objects/AnimObj.h>
#include "../io/ActionMgr.h"
#include <cstddef>
#include <vector>

class GuardEnemy;
class HealthPickup;
class Tilemap;

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
	struct CollisionBatch
	{
		std::vector<GObj*> tileSolids{};
		std::vector<HealthPickup*> healthPickups{};
		std::vector<GuardEnemy*> enemies{};
	};

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
	void beginFrame();
	void handleInput();
	void update(float dt, std::size_t activeShotCount, std::size_t maxShots);

	template <typename... EntityVectors>
	CollisionBatch CheckAndStoreCollisions(Tilemap& tilemap, EntityVectors&... entityVectors)
	{
		CollisionBatch collisions;
		checkTileCollisions(tilemap, collisions);
		(checkEntityCollisions(entityVectors, collisions), ...);
		return collisions;
	}

	void ResolveClosestCollisionsFirst(const CollisionBatch& collisions);
	void SwapData();
	bool consumeShotRequest();

	void resetCombatState();
	void updateCombatTimers(float dt);
	bool tryStartShot(std::size_t activeShotCount, std::size_t maxShots);
	sf::Vector2f getShotSpawnPosition(float shotRadius);
	PlayerJumpAnimPhase updateJumpAnimationPhase(bool wasGrounded, float dt, float jumpVelocity, float gravityPerFrame);
	bool takeDamage(int damage);
	bool takeDamage(int damage, sf::Vector2f damageSource);
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
	void clearCopyContacts();
	bool isDamageLocked() const;
	bool isHorizontalMovementLocked() const;
	bool canStartDash() const;
	void clearDashState();
	void clearWallState();
	void clearInputIntent();
	void updateMobilityTimers(float dt);
	void applyAbilityMovement();
	void startDash(int direction);
	void beginDashEnd();
	void startWallKick(int wallSide);
	void updateWallInteractionState();
	int getWallSideFromContacts() const;
	bool isPressingTowardWall(int wallSide) const;
	sf::Vector2f getCenter() const;
	void checkTileCollisions(Tilemap& tilemap, CollisionBatch& collisions) const;
	void checkEntityCollisions(std::vector<HealthPickup>& pickups, CollisionBatch& collisions) const;
	void checkEntityCollisions(std::vector<GuardEnemy>& enemies, CollisionBatch& collisions) const;

	template <typename T>
	void checkEntityCollisions(T&, CollisionBatch&) const
	{
	}

	ActionIntent buildIntent() const;
	ActionContext buildActionContext(EntityAction forcedAction) const;
	EntityAction actionFromJumpPhase(PlayerJumpAnimPhase phase) const;
	void finishAnimationFrame();

	ActionMgr actionMgr{};
	std::vector<DamagePop> damagePops{};
	float shotCooldown{ 0.0f };
	float shootPoseTimer{ 0.0f };
	float invincibleTimer{ 0.0f };
	float hitFlashTimer{ 0.0f };
	float hitStunTimer{ 0.0f };
	float hitKnockbackTimer{ 0.0f };
	float hitKnockbackVelocityX{ 0.0f };
	float dashStartTimer{ 0.0f };
	float dashActiveTimer{ 0.0f };
	float dashEndTimer{ 0.0f };
	int dashDirection{ 1 };
	float wallKickTimer{ 0.0f };
	int wallKickDirection{ 0 };
	float wallLandingTimer{ 0.0f };
	float wallReattachLockTimer{ 0.0f };
	int wallDetachSide{ 0 };
	int activeWallSide{ 0 };
	float wallDetachReferenceX{ 0.0f };
	float frameDt{ 0.0f };
	bool wasGroundedAtFrameStart{ false };
	bool shotSpawnRequested{ false };
};
