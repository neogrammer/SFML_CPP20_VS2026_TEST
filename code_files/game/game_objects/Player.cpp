#include "Player.h"
#include "CombatEntities.h"
#include "../Physics.h"
#include "../map/Tilemap.h"
#include <res/Cfg.h>

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float PlayerShotDelay = 0.16f;

	// Keep the arm-cannon pose alive after each shot. While this timer is
	// running, repeated shots do not restart from the "pull gun out" frame.
	constexpr float PlayerShootPoseSeconds = 0.55f;
	constexpr float HitFlashSeconds = 0.12f;
	constexpr float PlayerInvincibleSeconds = 1.55f;
	constexpr float DamageKnockbackSeconds = 0.18f;
	constexpr float DamageStunSeconds = DamageKnockbackSeconds;
	constexpr float DamageKnockbackSpeed = 320.0f;
	constexpr int EnemyBodyDamage = 2;
	constexpr AnimName DamagePoseAnim = AnimName::WallLand;
	constexpr float GroundRunVelocity = 500.0f;
	constexpr float AirRunVelocity = 620.0f;
	constexpr float JumpHangTimeScale = 1.45f;
	constexpr float JumpHeightBoost = 1.04f;
	constexpr float GravityPerFrame = 175.0f / (JumpHangTimeScale * JumpHangTimeScale);
	constexpr float JumpVelocity = (-2475.0f * JumpHeightBoost) / JumpHangTimeScale;
	constexpr float GroundProbeDistance = 1.0f;
	constexpr float SweepSkin = 0.05f;
	constexpr int MaxSweepPasses = 6;
	constexpr int HealthPickupAmount = 5;
	constexpr float LiftOffToRisingPercent = 0.10f;
	constexpr float JumpPeakBeforeTopPercent = 0.05f;
	constexpr float FallingAfterPeakPercent = 0.05f;

	struct SweepHit
	{
		GObj* solid{ nullptr };
		sf::Vector2f point{};
		sf::Vector2f normal{};
		float time{ 1.0f };
		float centerDistanceSq{ 0.0f };
	};

	bool isLandingAnim(AnimName anim)
	{
		return anim == AnimName::Landing || anim == AnimName::LandingShoot;
	}

	float estimateJumpHeight(float dt, float jumpVelocity, float gravityPerFrame)
	{
		float velocityY = jumpVelocity;
		float height = 0.0f;

		// Movement uses acceleration as a per-update velocity change, so the
		// estimate mirrors GObj::update() instead of continuous physics.
		for (int i = 0; i < 120; ++i)
		{
			velocityY += gravityPerFrame;
			if (velocityY >= 0.0f)
			{
				break;
			}

			height += -velocityY * dt;
		}

		return std::max(height, 1.0f);
	}

	bool isShootAnim(AnimName anim)
	{
		return anim == AnimName::Shoot ||
			anim == AnimName::RunShoot ||
			anim == AnimName::LiftOffShoot ||
			anim == AnimName::RisingShoot ||
			anim == AnimName::JumpPeakShoot ||
			anim == AnimName::FallingShoot ||
			anim == AnimName::LandingShoot ||
			anim == AnimName::DashStartShoot ||
			anim == AnimName::DashingShoot ||
			anim == AnimName::DashEndShoot ||
			anim == AnimName::WallKickShoot ||
			anim == AnimName::WallLandShoot ||
			anim == AnimName::WallSlideShoot;
	}

	float distanceSq(sf::Vector2f a, sf::Vector2f b)
	{
		const float dx = a.x - b.x;
		const float dy = a.y - b.y;
		return (dx * dx) + (dy * dy);
	}

	sf::Vector2f centerOf(const GObj& obj)
	{
		return {
			obj.getPosSafe().x + (obj.getSizeSafe().x * 0.5f),
			obj.getPosSafe().y + (obj.getSizeSafe().y * 0.5f)
		};
	}

	bool boxesOverlap(const Physics::Box& first, const Physics::Box& second)
	{
		if (first.width <= 0.0f || first.height <= 0.0f || second.width <= 0.0f || second.height <= 0.0f)
		{
			return false;
		}

		return first.left < second.right() &&
			first.right() > second.left &&
			first.top < second.bottom() &&
			first.bottom() > second.top;
	}

	bool findClosestCurrentSweepHit(GObj& mover, const std::vector<GObj*>& solids, float dt, SweepHit& closest)
	{
		bool found = false;
		const sf::Vector2f moverCenter = centerOf(mover);

		for (GObj* solid : solids)
		{
			if (solid == nullptr)
			{
				continue;
			}

			SweepHit hit;
			hit.solid = solid;

			if (!Physics::DynamicRectVsRect(&mover, dt, *solid, hit.point, hit.normal, hit.time))
			{
				continue;
			}

			if (hit.normal.x == 0.0f && hit.normal.y == 0.0f)
			{
				continue;
			}

			hit.centerDistanceSq = distanceSq(moverCenter, centerOf(*solid));

			const bool isEarlier = !found || hit.time < closest.time;
			const bool isSameTimeButCloser =
				found &&
				std::fabs(hit.time - closest.time) <= 0.0001f &&
				hit.centerDistanceSq < closest.centerDistanceSq;

			if (isEarlier || isSameTimeButCloser)
			{
				closest = hit;
				found = true;
			}
		}

		return found;
	}

	void markContactAndClipVelocity(Player& player, const SweepHit& hit, sf::Vector2f& velocity)
	{
		if (player.copy == nullptr || hit.solid == nullptr)
		{
			return;
		}

		if (hit.normal.y > 0.0f)
		{
			player.copy->contact[0] = hit.solid;
			velocity.y = 0.0f;
		}

		if (hit.normal.x < 0.0f)
		{
			player.copy->contact[1] = hit.solid;
			velocity.x = 0.0f;
		}

		if (hit.normal.y < 0.0f)
		{
			player.copy->contact[2] = hit.solid;
			player.copy->grounded = true;
			player.copy->setJustLanded(true);
			velocity.y = 0.0f;
		}

		if (hit.normal.x > 0.0f)
		{
			player.copy->contact[3] = hit.solid;
			velocity.x = 0.0f;
		}
	}

	void resolveSweptCollisions(Player& player, const std::vector<GObj*>& solids, float dt)
	{
		if (player.copy == nullptr)
		{
			return;
		}

		sf::Vector2f position = player.copy->getPosSafe();
		sf::Vector2f velocity = player.copy->getVelSafe();
		float remainingTime = dt;

		for (int pass = 0; pass < MaxSweepPasses && remainingTime > 0.000001f; ++pass)
		{
			player.copy->setPos(position);
			player.copy->setVel(velocity);

			SweepHit closest;
			if (!findClosestCurrentSweepHit(*player.copy, solids, remainingTime, closest))
			{
				position += velocity * remainingTime;
				remainingTime = 0.0f;
				break;
			}

			const float safeTime = std::clamp(closest.time, 0.0f, 1.0f);
			position += velocity * (remainingTime * safeTime);
			position += closest.normal * SweepSkin;

			markContactAndClipVelocity(player, closest, velocity);

			remainingTime *= (1.0f - safeTime);

			if (velocity.x == 0.0f && velocity.y == 0.0f)
			{
				break;
			}
		}

		player.copy->setPos(position);
		player.copy->setVel(velocity);
	}

	void snapToGroundIfStanding(Player& player, const std::vector<GObj*>& solids)
	{
		if (player.copy == nullptr || player.copy->grounded)
		{
			return;
		}

		const sf::Vector2f probePos{
			player.copy->getPosSafe().x,
			player.copy->getPosSafe().y + GroundProbeDistance
		};

		if (Physics::overlapsAnyAt(player.copy, solids, probePos))
		{
			player.copy->grounded = true;
			player.copy->setVel({ player.copy->getVelSafe().x, 0.0f });
		}
	}
}

Player::Player()
	: AnimObj(Cfg::Textures::PlayerAtlas, { {0,0},{192,192} }, false, { 400.f,400.f }, { 72.f,114.f }, {64.f,42.f})
{

}

Player::~Player()
{

}

const GObj& Player::operator()()
{
	return *this;
}

void Player::beginFrame()
{
	if (copy == nullptr)
	{
		return;
	}

	wasGroundedAtFrameStart = grounded;
	frameDt = 0.0f;
	shotSpawnRequested = false;

	syncCopyFromLive();
	copy->grounded = false;
	copy->setJustLanded(false);
	copy->justLeftGround = false;
	clearCopyContacts();
}

void Player::handleInput()
{
	if (copy == nullptr)
	{
		return;
	}

	if (isDamageLocked())
	{
		clearInputIntent();
		copy->setVel({ 0.0f, 0.0f });
		return;
	}

	const bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
	const bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
	const bool jump = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
	const bool shoot = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);

	rightHeld = right;
	leftHeld = left;

	jumpPressedThisFrame = jump && !jumpHeld;
	jumpHeld = jump;
	shootPressedThisFrame = shoot && !shootHeld;
	shootHeld = shoot;

	sf::Vector2f velocity = copy->getVelSafe();
	const float moveSpeed = grounded ? GroundRunVelocity : AirRunVelocity;

	if (right == left)
	{
		velocity.x = 0.0f;
	}
	else if (right)
	{
		velocity.x = moveSpeed;
		setFacingRightCpy(true);
	}
	else
	{
		velocity.x = -moveSpeed;
		setFacingRightCpy(false);
	}

	copy->setVel(velocity);
}

void Player::update(float dt, std::size_t activeShotCount, std::size_t maxShots)
{
	if (copy == nullptr)
	{
		return;
	}

	frameDt = dt;
	updateCombatTimers(dt);

	if (isDamageLocked())
	{
		clearInputIntent();

		sf::Vector2f velocity{ 0.0f, 0.0f };
		if (hitKnockbackTimer > 0.0f)
		{
			velocity.x = hitKnockbackVelocityX;
		}

		copy->setVel(velocity);
		copy->setAccleration({ 0.0f, 0.0f });
		updatePhysics(dt);
		shotSpawnRequested = false;
		return;
	}

	if (jumpPressedThisFrame && wasGroundedAtFrameStart)
	{
		copy->setVel({ copy->getVelSafe().x, JumpVelocity });
		copy->justLeftGround = true;
	}

	const bool shouldApplyGravity =
		!wasGroundedAtFrameStart ||
		copy->getVelSafe().y < 0.0f ||
		copy->justLeftGround;

	copy->setAccleration({ 0.0f, shouldApplyGravity ? GravityPerFrame : 0.0f });
	updatePhysics(dt);

	shotSpawnRequested = tryStartShot(activeShotCount, maxShots);
}

void Player::ResolveClosestCollisionsFirst(const CollisionBatch& collisions)
{
	if (copy == nullptr)
	{
		return;
	}

	resolveSweptCollisions(*this, collisions.tileSolids, frameDt);
	Physics::moveFirstOutsideVector(this, collisions.tileSolids, true);
	snapToGroundIfStanding(*this, collisions.tileSolids);
	copy->setJustLanded(!wasGroundedAtFrameStart && copy->grounded);

	const Physics::Box playerBox = Physics::getBox(copy);
	for (HealthPickup* pickup : collisions.healthPickups)
	{
		if (pickup == nullptr || !pickup->canBeCollected())
		{
			continue;
		}

		if (!boxesOverlap(playerBox, pickup->getCollisionBox()))
		{
			continue;
		}

		heal(HealthPickupAmount);
		pickup->markCollected();
	}

	for (GuardEnemy* enemy : collisions.enemies)
	{
		if (enemy == nullptr || !enemy->willBeAlive())
		{
			continue;
		}

		if (takeDamage(EnemyBodyDamage, enemy->getCenter()))
		{
			break;
		}
	}

	finishAnimationFrame();
}

void Player::SwapData()
{
	swapdate();
}

bool Player::consumeShotRequest()
{
	const bool out = shotSpawnRequested;
	shotSpawnRequested = false;
	return out;
}

void Player::resetCombatState()
{
	health = maxHealth;
	weaponIsHoldingShootPose = false;
	damagePops.clear();
	shotCooldown = 0.0f;
	shootPoseTimer = 0.0f;
	invincibleTimer = 0.0f;
	hitFlashTimer = 0.0f;
	hitStunTimer = 0.0f;
	hitKnockbackTimer = 0.0f;
	hitKnockbackVelocityX = 0.0f;
}

void Player::updateCombatTimers(float dt)
{
	shotCooldown = std::max(0.0f, shotCooldown - dt);
	shootPoseTimer = std::max(0.0f, shootPoseTimer - dt);
	invincibleTimer = std::max(0.0f, invincibleTimer - dt);
	hitFlashTimer = std::max(0.0f, hitFlashTimer - dt);
	hitStunTimer = std::max(0.0f, hitStunTimer - dt);
	hitKnockbackTimer = std::max(0.0f, hitKnockbackTimer - dt);

	for (DamagePop& pop : damagePops)
	{
		pop.timer -= dt;
		pop.position += pop.velocity * dt;
	}

	damagePops.erase(
		std::remove_if(damagePops.begin(), damagePops.end(), [](const DamagePop& pop) { return pop.timer <= 0.0f; }),
		damagePops.end()
	);

	weaponIsHoldingShootPose = shootPoseTimer > 0.0f;

	if (isDamageLocked())
	{
		weaponIsHoldingShootPose = false;
	}
}

bool Player::tryStartShot(std::size_t activeShotCount, std::size_t maxShots)
{
	if (!shootPressedThisFrame)
	{
		return false;
	}

	const bool shotLimitReached = activeShotCount >= maxShots;
	const bool shotDelayActive = shotCooldown > 0.0f;
	if (shotLimitReached || shotDelayActive)
	{
		shootPressedThisFrame = false;
		return false;
	}

	shotCooldown = PlayerShotDelay;
	shootPoseTimer = PlayerShootPoseSeconds;
	weaponIsHoldingShootPose = true;
	return true;
}

sf::Vector2f Player::getShotSpawnPosition(float shotRadius)
{
	const bool right = isFacingRight();
	const AnimName anim = getCurrentAnim();
	const int frame = getCurrentIndex();
	const sf::Vector2f currentSize = getSizeSafe();

	float x = right ? currentSize.x + 10.0f : -10.0f;
	float y = currentSize.y * 0.42f;

	if (anim == AnimName::RunShoot)
	{
		const float bob[10]{ 0.0f, -2.0f, -4.0f, -3.0f, 1.0f, 3.0f, 2.0f, 0.0f, -1.0f, 1.0f };
		y += bob[frame % 10];
		x += right ? 4.0f : -4.0f;
	}
	else if (anim == AnimName::LiftOffShoot || anim == AnimName::RisingShoot || anim == AnimName::JumpPeakShoot)
	{
		y = currentSize.y * 0.34f;
	}
	else if (anim == AnimName::FallingShoot)
	{
		y = currentSize.y * 0.45f;
	}
	else if (anim == AnimName::LandingShoot)
	{
		y = currentSize.y * 0.52f;
	}
	else if (!isShootAnim(anim))
	{
		y = currentSize.y * 0.40f;
	}

	sf::Vector2f out = getPosSafe() + sf::Vector2f{ x, y };
	out.y -= shotRadius * 2.0f;
	return out;
}

PlayerJumpAnimPhase Player::updateJumpAnimationPhase(bool wasGrounded, float dt, float jumpVelocity, float gravityPerFrame)
{
	if (copy == nullptr)
	{
		return PlayerJumpAnimPhase::Grounded;
	}

	if (copy->grounded)
	{
		if (!wasGrounded)
		{
			jumpAnimPhase = PlayerJumpAnimPhase::Landing;
		}

		if (jumpAnimPhase == PlayerJumpAnimPhase::Landing)
		{
			const AnimName currentAnim = getCurrentAnim();
			const bool landingAlreadyPlaying = isLandingAnim(currentAnim);
			if (!landingAlreadyPlaying || !isCurrentAnimationFinished())
			{
				return PlayerJumpAnimPhase::Landing;
			}
		}

		jumpAnimPhase = PlayerJumpAnimPhase::Grounded;
		jumpAnimFramesInAir = 0;
		return PlayerJumpAnimPhase::Grounded;
	}

	if (copy->justLeftGround || jumpAnimPhase == PlayerJumpAnimPhase::Grounded)
	{
		jumpAnimPhase = copy->getVelSafe().y < 0.0f
			? PlayerJumpAnimPhase::LiftOff
			: PlayerJumpAnimPhase::Falling;

		jumpStartY = getPosSafe().y;
		jumpPeakY = copy->getPosSafe().y;
		jumpExpectedHeight = estimateJumpHeight(dt, jumpVelocity, gravityPerFrame);
		jumpAnimFramesInAir = 0;
	}

	jumpAnimFramesInAir++;

	if (copy->getPosSafe().y < jumpPeakY)
	{
		jumpPeakY = copy->getPosSafe().y;
	}

	const float expectedHeight = std::max(jumpExpectedHeight, 1.0f);
	const float traveledUp = std::max(0.0f, jumpStartY - copy->getPosSafe().y);
	const float leftUntilExpectedTop = std::max(0.0f, expectedHeight - traveledUp);
	const float downFromPeak = std::max(0.0f, copy->getPosSafe().y - jumpPeakY);

	if (jumpAnimPhase == PlayerJumpAnimPhase::LiftOff)
	{
		const bool hasShownLiftOffForAFrame = jumpAnimFramesInAir > 1;
		if (hasShownLiftOffForAFrame && traveledUp >= expectedHeight * LiftOffToRisingPercent)
		{
			jumpAnimPhase = PlayerJumpAnimPhase::Rising;
		}
	}

	if (jumpAnimPhase == PlayerJumpAnimPhase::Rising)
	{
		const bool isNearTop = leftUntilExpectedTop <= expectedHeight * JumpPeakBeforeTopPercent;
		const bool velocityHasReachedTop = copy->getVelSafe().y >= 0.0f;
		if (isNearTop || velocityHasReachedTop)
		{
			jumpAnimPhase = PlayerJumpAnimPhase::JumpPeak;
		}
	}

	if (jumpAnimPhase == PlayerJumpAnimPhase::JumpPeak)
	{
		const bool isComingDown = copy->getVelSafe().y > 0.0f;
		const bool hasDroppedEnough = downFromPeak >= expectedHeight * FallingAfterPeakPercent;
		if (isComingDown && hasDroppedEnough)
		{
			jumpAnimPhase = PlayerJumpAnimPhase::Falling;
		}
	}

	return jumpAnimPhase;
}

bool Player::takeDamage(int damage)
{
	return takeDamage(damage, getCenter() + sf::Vector2f{ isFacingRight() ? 1.0f : -1.0f, 0.0f });
}

bool Player::takeDamage(int damage, sf::Vector2f damageSource)
{
	if (invincibleTimer > 0.0f)
	{
		return false;
	}

	health = std::max(0, health - damage);
	invincibleTimer = PlayerInvincibleSeconds;
	hitFlashTimer = HitFlashSeconds;
	hitStunTimer = DamageStunSeconds;
	hitKnockbackTimer = DamageKnockbackSeconds;

	const sf::Vector2f center = getCenter();
	const bool sourceIsLeft = damageSource.x < center.x;
	hitKnockbackVelocityX = sourceIsLeft ? DamageKnockbackSpeed : -DamageKnockbackSpeed;

	if (copy != nullptr)
	{
		copy->setFacingRight(!sourceIsLeft);
		copy->setVel({ hitKnockbackVelocityX, 0.0f });
	}
	else
	{
		setFacingRight(!sourceIsLeft);
	}

	spawnHealthDamagePop(damage);
	return true;
}

void Player::heal(int amount)
{
	health = std::min(maxHealth, health + amount);
}

bool Player::shouldBlinkOff() const
{
	return invincibleTimer > 0.0f &&
		hitFlashTimer <= 0.0f &&
		(static_cast<int>(invincibleTimer / 0.08f) % 2 == 0);
}

bool Player::isHitFlashActive() const
{
	return hitFlashTimer > 0.0f;
}

const std::vector<Player::DamagePop>& Player::getDamagePops() const
{
	return damagePops;
}

void Player::spawnHealthDamagePop(int damage)
{
	DamagePop pop;
	pop.amount = damage;
	pop.timer = pop.lifetime;

	const float segmentWidth = 12.0f;
	const float gap = 2.0f;
	const int hurtSegment = std::clamp(health, 0, maxHealth - 1);
	pop.position = {
		22.0f + (hurtSegment * (segmentWidth + gap)),
		48.0f
	};
	pop.velocity = { 18.0f,  -52.0f };

	damagePops.emplace_back(pop);
}

void Player::clearCopyContacts()
{
	if (copy == nullptr)
	{
		return;
	}

	for (GObj*& contact : copy->contact)
	{
		contact = nullptr;
	}
}

bool Player::isDamageLocked() const
{
	return hitStunTimer > 0.0f;
}

void Player::clearInputIntent()
{
	movingRight = false;
	movingLeft = false;
	leftHeld = false;
	rightHeld = false;
	jumpPressedThisFrame = false;
	jumpHeld = false;
	dashPressedThisFrame = false;
	dashHeld = false;
	shootPressedThisFrame = false;
	shootHeld = false;
}

sf::Vector2f Player::getCenter() const
{
	return {
		getPosSafe().x + (getSizeSafe().x * 0.5f),
		getPosSafe().y + (getSizeSafe().y * 0.5f)
	};
}

void Player::checkTileCollisions(Tilemap& tilemap, CollisionBatch& collisions) const
{
	collisions.tileSolids = tilemap.getSolids();
}

void Player::checkEntityCollisions(std::vector<HealthPickup>& pickups, CollisionBatch& collisions) const
{
	if (copy == nullptr)
	{
		return;
	}

	const Physics::Box playerBox = Physics::getBox(copy);
	for (HealthPickup& pickup : pickups)
	{
		if (!pickup.canBeCollected())
		{
			continue;
		}

		if (boxesOverlap(playerBox, pickup.getCollisionBox()))
		{
			collisions.healthPickups.emplace_back(&pickup);
		}
	}
}

void Player::checkEntityCollisions(std::vector<GuardEnemy>& enemies, CollisionBatch& collisions) const
{
	if (copy == nullptr)
	{
		return;
	}

	const Physics::Box playerBox = Physics::getBox(copy);
	for (GuardEnemy& enemy : enemies)
	{
		if (!enemy.willBeAlive())
		{
			continue;
		}

		if (boxesOverlap(playerBox, enemy.getCollisionBox()))
		{
			collisions.enemies.emplace_back(&enemy);
		}
	}
}

ActionIntent Player::buildIntent() const
{
	ActionIntent intent;

	if (leftHeld)
	{
		intent.moveX -= 1.0f;
	}

	if (rightHeld)
	{
		intent.moveX += 1.0f;
	}

	intent.jumpPressed = jumpPressedThisFrame;
	intent.jumpHeld = jumpHeld;
	intent.dashPressed = dashPressedThisFrame;
	intent.dashHeld = dashHeld;
	intent.shootPressed = shootPressedThisFrame;
	intent.shootHeld = false;

	return intent;
}

ActionContext Player::buildActionContext(EntityAction forcedAction) const
{
	const GObj* state = (copy != nullptr) ? copy : this;

	ActionContext context;
	context.forcedAction = forcedAction;
	context.velocity = state->getVelSafe();
	context.grounded = state->grounded;
	context.justLeftGround = state->justLeftGround;
	context.justLanded = state->justLanded;

	context.dashStarting = dashStarting;
	context.dashing = dashing;
	context.dashEnding = dashEnding;

	context.wallKicking = wallKicking;
	context.wallLanding = wallLanding;
	context.wallSliding = wallSliding;

	context.teleportingIn = teleportingIn;
	context.teleportLanding = teleportLanding;

	context.shootingLocked = weaponIsHoldingShootPose;

	return context;
}

EntityAction Player::actionFromJumpPhase(PlayerJumpAnimPhase phase) const
{
	switch (phase)
	{
	case PlayerJumpAnimPhase::LiftOff:  return EntityAction::LiftOff;
	case PlayerJumpAnimPhase::Rising:   return EntityAction::Rising;
	case PlayerJumpAnimPhase::JumpPeak: return EntityAction::JumpPeak;
	case PlayerJumpAnimPhase::Falling:  return EntityAction::Falling;
	case PlayerJumpAnimPhase::Landing:  return EntityAction::Landing;
	default:                            return EntityAction::None;
	}
}

void Player::finishAnimationFrame()
{
	if (isDamageLocked())
	{
		setCurrentAnim(DamagePoseAnim);
		return;
	}

	const ActionIntent intent = buildIntent();
	const PlayerJumpAnimPhase jumpPhase = updateJumpAnimationPhase(
		wasGroundedAtFrameStart,
		frameDt,
		JumpVelocity,
		GravityPerFrame
	);
	const EntityAction jumpAction = actionFromJumpPhase(jumpPhase);
	const ActionContext context = buildActionContext(jumpAction);

	actionMgr.update(*this, intent, context);
	updateAnimation(frameDt);
}
