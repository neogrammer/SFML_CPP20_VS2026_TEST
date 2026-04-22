#include "Player.h"
#include <res/Cfg.h>

#include <algorithm>

namespace
{
	constexpr float PlayerShotDelay = 0.16f;

	// Keep the arm-cannon pose alive after each shot. While this timer is
	// running, repeated shots do not restart from the "pull gun out" frame.
	constexpr float PlayerShootPoseSeconds = 0.55f;
	constexpr float HitFlashSeconds = 0.12f;
	constexpr float PlayerInvincibleSeconds = 1.15f;
	constexpr float LiftOffToRisingPercent = 0.10f;
	constexpr float JumpPeakBeforeTopPercent = 0.05f;
	constexpr float FallingAfterPeakPercent = 0.05f;

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

void Player::resetCombatState()
{
	health = maxHealth;
	weaponIsHoldingShootPose = false;
	damagePops.clear();
	shotCooldown = 0.0f;
	shootPoseTimer = 0.0f;
	invincibleTimer = 0.0f;
	hitFlashTimer = 0.0f;
}

void Player::updateCombatTimers(float dt)
{
	shotCooldown = std::max(0.0f, shotCooldown - dt);
	shootPoseTimer = std::max(0.0f, shootPoseTimer - dt);
	invincibleTimer = std::max(0.0f, invincibleTimer - dt);
	hitFlashTimer = std::max(0.0f, hitFlashTimer - dt);

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
	if (invincibleTimer > 0.0f)
	{
		return false;
	}

	health = std::max(0, health - damage);
	invincibleTimer = PlayerInvincibleSeconds;
	hitFlashTimer = HitFlashSeconds;
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
