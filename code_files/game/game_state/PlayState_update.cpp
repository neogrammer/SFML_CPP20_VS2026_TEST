#include "PlayState.h"
#include "../Physics.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

namespace
{
    // Jump feel lives here: velocity is the launch, gravity decides hang time.
    // Softer gravity keeps the player in the air longer without touching collision.
    constexpr float GravityPerFrame = 175.0f;
    constexpr float JumpVelocity = -2475.0f;
    constexpr float GroundProbeDistance = 1.0f;
    constexpr float SweepSkin = 0.05f;
    constexpr float LiftOffToRisingPercent = 0.10f;
    constexpr float JumpPeakBeforeTopPercent = 0.05f;
    constexpr float FallingAfterPeakPercent = 0.05f;
    constexpr int MaxSweepPasses = 6;
    constexpr int MaxPlayerShots = 3;
    constexpr float PlayerShotDelay = 0.16f;
    // Keep the arm-cannon pose alive after each shot. While this timer is
    // running, repeated shots do not restart from the "pull gun out" frame.
    constexpr float PlayerShootPoseSeconds = 0.55f;
    constexpr float PlayerShotSpeed = 1350.0f;
    constexpr float EnemyShotSpeed = 920.0f;
    constexpr float EnemyShotDelay = 2.5f;
    constexpr float EnemySightDistance = 600.0f;
    constexpr float EnemySightHeight = 120.0f;
    constexpr float EnemyWalkSpeed = 120.0f;
    constexpr int EnemyShotDamage = 2;
    constexpr float HitFlashSeconds = 0.12f;
    constexpr float PlayerInvincibleSeconds = 1.15f;
    constexpr int HealthPickupAmount = 5;
    constexpr float HealthPickupGravity = 1450.0f;

    struct SweepHit
    {
        GObj* solid{ nullptr };
        sf::Vector2f point{};
        sf::Vector2f normal{};
        float time{ 1.0f };
        float centerDistanceSq{ 0.0f };
    };

    void clearContacts(GObj& obj)
    {
        for (GObj*& contact : obj.contact)
        {
            contact = nullptr;
        }
    }

    ActionIntent buildPlayerIntent(const Player& player)
    {
        ActionIntent intent;

        if (player.leftHeld)
        {
            intent.moveX -= 1.0f;
        }

        if (player.rightHeld)
        {
            intent.moveX += 1.0f;
        }

        intent.jumpPressed = player.jumpPressedThisFrame;
        intent.jumpHeld = player.jumpHeld;
        intent.dashPressed = player.dashPressedThisFrame;
        intent.dashHeld = player.dashHeld;
        intent.shootPressed = player.shootPressedThisFrame;
        // Enter is treated as a shot press. The short shoot pose is kept alive
        // by weaponIsHoldingShootPose, so holding Enter does not lock shooting.
        intent.shootHeld = false;

        return intent;
    }

    ActionContext buildPlayerContext(const Player& player, EntityAction forcedAction)
    {
        const GObj* state = (player.copy != nullptr) ? player.copy : &player;

        ActionContext context;
        context.forcedAction = forcedAction;
        context.velocity = state->getVelSafe();
        context.grounded = state->grounded;
        context.justLeftGround = state->justLeftGround;
        context.justLanded = state->justLanded;

        context.dashStarting = player.dashStarting;
        context.dashing = player.dashing;
        context.dashEnding = player.dashEnding;

        context.wallKicking = player.wallKicking;
        context.wallLanding = player.wallLanding;
        context.wallSliding = player.wallSliding;

        context.teleportingIn = player.teleportingIn;
        context.teleportLanding = player.teleportLanding;

        context.shootingLocked = player.weaponIsHoldingShootPose;

        return context;
    }

    bool isLandingAnim(AnimName anim)
    {
        return anim == AnimName::Landing || anim == AnimName::LandingShoot;
    }

    float estimateJumpHeight(float dt)
    {
        float velocityY = JumpVelocity;
        float height = 0.0f;

        // Your movement uses acceleration as a per-update velocity change,
        // so this mirrors updatePhysics() instead of using continuous physics.
        for (int i = 0; i < 120; ++i)
        {
            velocityY += GravityPerFrame;
            if (velocityY >= 0.0f)
            {
                break;
            }

            height += -velocityY * dt;
        }

        return std::max(height, 1.0f);
    }

    EntityAction actionFromJumpPhase(PlayerJumpAnimPhase phase)
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

    EntityAction updateJumpAnimationPhase(Player& player, bool wasGrounded, float dt)
    {
        if (player.copy == nullptr)
        {
            return EntityAction::None;
        }

        if (player.copy->grounded)
        {
            if (!wasGrounded)
            {
                player.jumpAnimPhase = PlayerJumpAnimPhase::Landing;
            }

            if (player.jumpAnimPhase == PlayerJumpAnimPhase::Landing)
            {
                const AnimName currentAnim = player.getCurrentAnim();
                const bool landingAlreadyPlaying = isLandingAnim(currentAnim);
                if (!landingAlreadyPlaying || !player.isCurrentAnimationFinished())
                {
                    return EntityAction::Landing;
                }
            }

            player.jumpAnimPhase = PlayerJumpAnimPhase::Grounded;
            player.jumpAnimFramesInAir = 0;
            return EntityAction::None;
        }

        if (player.copy->justLeftGround || player.jumpAnimPhase == PlayerJumpAnimPhase::Grounded)
        {
            player.jumpAnimPhase = player.copy->getVelSafe().y < 0.0f
                ? PlayerJumpAnimPhase::LiftOff
                : PlayerJumpAnimPhase::Falling;

            player.jumpStartY = player.getPosSafe().y;
            player.jumpPeakY = player.copy->getPosSafe().y;
            player.jumpExpectedHeight = estimateJumpHeight(dt);
            player.jumpAnimFramesInAir = 0;
        }

        player.jumpAnimFramesInAir++;

        if (player.copy->getPosSafe().y < player.jumpPeakY)
        {
            player.jumpPeakY = player.copy->getPosSafe().y;
        }

        const float expectedHeight = std::max(player.jumpExpectedHeight, 1.0f);
        const float traveledUp = std::max(0.0f, player.jumpStartY - player.copy->getPosSafe().y);
        const float leftUntilExpectedTop = std::max(0.0f, expectedHeight - traveledUp);
        const float downFromPeak = std::max(0.0f, player.copy->getPosSafe().y - player.jumpPeakY);

        if (player.jumpAnimPhase == PlayerJumpAnimPhase::LiftOff)
        {
            const bool hasShownLiftOffForAFrame = player.jumpAnimFramesInAir > 1;
            if (hasShownLiftOffForAFrame && traveledUp >= expectedHeight * LiftOffToRisingPercent)
            {
                player.jumpAnimPhase = PlayerJumpAnimPhase::Rising;
            }
        }

        if (player.jumpAnimPhase == PlayerJumpAnimPhase::Rising)
        {
            const bool isNearTop = leftUntilExpectedTop <= expectedHeight * JumpPeakBeforeTopPercent;
            const bool velocityHasReachedTop = player.copy->getVelSafe().y >= 0.0f;
            if (isNearTop || velocityHasReachedTop)
            {
                player.jumpAnimPhase = PlayerJumpAnimPhase::JumpPeak;
            }
        }

        if (player.jumpAnimPhase == PlayerJumpAnimPhase::JumpPeak)
        {
            const bool isComingDown = player.copy->getVelSafe().y > 0.0f;
            const bool hasDroppedEnough = downFromPeak >= expectedHeight * FallingAfterPeakPercent;
            if (isComingDown && hasDroppedEnough)
            {
                player.jumpAnimPhase = PlayerJumpAnimPhase::Falling;
            }
        }

        return actionFromJumpPhase(player.jumpAnimPhase);
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

        // Keep asking "what is the closest thing I hit from where I am now?"
        // After every hit, the copy position and velocity are updated before
        // scanning again, so stale hits from the old path naturally drop out.
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

    Physics::Box makeBox(const sf::Vector2f& position, const sf::Vector2f& size)
    {
        Physics::Box box;
        box.left = position.x;
        box.top = position.y;
        box.width = size.x;
        box.height = size.y;
        return box;
    }

    Physics::Box makeShotBox(const PlayState::EnergyShot& shot)
    {
        return makeBox(
            { shot.position.x - shot.radius, shot.position.y - shot.radius },
            { shot.radius * 2.0f, shot.radius * 2.0f }
        );
    }

    Physics::Box makePickupBox(const PlayState::HealthPickup& pickup)
    {
        return makeBox(
            { pickup.position.x - pickup.radius, pickup.position.y - pickup.radius },
            { pickup.radius * 2.0f, pickup.radius * 2.0f }
        );
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

    sf::Vector2f boxCenter(const sf::Vector2f& position, const sf::Vector2f& size)
    {
        return { position.x + (size.x * 0.5f), position.y + (size.y * 0.5f) };
    }

    sf::Vector2f enemySize(const PlayState::GuardEnemy& enemy)
    {
        return enemy.guarding ? enemy.guardingSize : enemy.standingSize;
    }

    sf::Vector2f enemyCenter(const PlayState::GuardEnemy& enemy)
    {
        return boxCenter(enemy.position, enemySize(enemy));
    }

    sf::Vector2f playerCenter(const Player& player)
    {
        return boxCenter(player.getPosSafe(), player.getSizeSafe());
    }

    sf::Vector2f normalized(sf::Vector2f value)
    {
        const float lengthSq = (value.x * value.x) + (value.y * value.y);
        if (lengthSq <= 0.0001f)
        {
            return { 1.0f, 0.0f };
        }

        const float length = std::sqrt(lengthSq);
        return { value.x / length, value.y / length };
    }

    bool pointInsideBox(sf::Vector2f point, const Physics::Box& box)
    {
        return point.x >= box.left &&
            point.x <= box.right() &&
            point.y >= box.top &&
            point.y <= box.bottom();
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

    sf::Vector2f cannonOffsetFor(Player& player)
    {
        const bool right = player.isFacingRight();
        const AnimName anim = player.getCurrentAnim();
        const int frame = player.getCurrentIndex();
        const sf::Vector2f size = player.getSizeSafe();

        float x = right ? size.x + 10.0f : -10.0f;
        float y = size.y * 0.42f;

        if (anim == AnimName::RunShoot)
        {
            const float bob[10]{ 0.0f, -2.0f, -4.0f, -3.0f, 1.0f, 3.0f, 2.0f, 0.0f, -1.0f, 1.0f };
            y += bob[frame % 10];
            x += right ? 4.0f : -4.0f;
        }
        else if (anim == AnimName::LiftOffShoot || anim == AnimName::RisingShoot || anim == AnimName::JumpPeakShoot)
        {
            y = size.y * 0.34f;
        }
        else if (anim == AnimName::FallingShoot)
        {
            y = size.y * 0.45f;
        }
        else if (anim == AnimName::LandingShoot)
        {
            y = size.y * 0.52f;
        }
        else if (!isShootAnim(anim))
        {
            y = size.y * 0.40f;
        }

        return { x, y };
    }
}

PlayState::PlayState(sf::RenderWindow& wnd)
    : GameState<PlayState>{}
{
    mainView = wnd.getDefaultView();
}

void PlayState::updateCombatTimers(float dt)
{
    // Shooting is press-based. This timer just keeps the shoot pose visible
    // long enough for the animation to read like an intentional action.
    mPlayerShotCooldown = std::max(0.0f, mPlayerShotCooldown - dt);
    mPlayerShootPoseTimer = std::max(0.0f, mPlayerShootPoseTimer - dt);
    mPlayerInvincibleTimer = std::max(0.0f, mPlayerInvincibleTimer - dt);
    mPlayerHitFlashTimer = std::max(0.0f, mPlayerHitFlashTimer - dt);

    for (DamagePop& pop : mDamagePops)
    {
        pop.timer -= dt;
        pop.position += pop.velocity * dt;
    }

    mDamagePops.erase(
        std::remove_if(mDamagePops.begin(), mDamagePops.end(), [](const DamagePop& pop) { return pop.timer <= 0.0f; }),
        mDamagePops.end()
    );

    if (player != nullptr)
    {
        player->weaponIsHoldingShootPose = mPlayerShootPoseTimer > 0.0f;
    }
}

void PlayState::spawnHealthDamagePop(int damage)
{
    if (player == nullptr)
    {
        return;
    }

    DamagePop pop;
    pop.amount = damage;
    pop.timer = pop.lifetime;

    const float segmentWidth = 12.0f;
    const float gap = 2.0f;
    const int hurtSegment = std::clamp(player->health, 0, player->maxHealth - 1);
    pop.position = {
        22.0f + (hurtSegment * (segmentWidth + gap)),
        48.0f
    };
    pop.velocity = { 18.0f,  -52.0f };

    mDamagePops.emplace_back(pop);
}

bool PlayState::damagePlayer(int damage)
{
    if (player == nullptr || mPlayerInvincibleTimer > 0.0f)
    {
        return false;
    }

    player->health = std::max(0, player->health - damage);
    mPlayerInvincibleTimer = PlayerInvincibleSeconds;
    mPlayerHitFlashTimer = HitFlashSeconds;
    spawnHealthDamagePop(damage);
    return true;
}

void PlayState::spawnHealthPickup(sf::Vector2f position)
{
    HealthPickup pickup;
    pickup.position = position;
    pickup.velocity = { 0.0f, -160.0f };
    mHealthPickups.emplace_back(pickup);
}

void PlayState::updateHealthPickups(float dt)
{
    if (player == nullptr || tmap == nullptr)
    {
        return;
    }

    for (HealthPickup& pickup : mHealthPickups)
    {
        if (!pickup.alive)
        {
            continue;
        }

        if (!pickup.settled)
        {
            pickup.velocity.y += HealthPickupGravity * dt;
            pickup.position += pickup.velocity * dt;

            Physics::Box pickupBox = makePickupBox(pickup);
            for (GObj* solid : tmap->getSolids())
            {
                if (solid == nullptr)
                {
                    continue;
                }

                const Physics::Box tileBox = Physics::getBox(solid);
                const bool fallingOntoTile =
                    pickup.velocity.y >= 0.0f &&
                    pickupBox.bottom() >= tileBox.top &&
                    pickupBox.top < tileBox.top &&
                    pickupBox.right() > tileBox.left &&
                    pickupBox.left < tileBox.right();

                if (fallingOntoTile)
                {
                    pickup.position.y = tileBox.top - pickup.radius - 1.0f;
                    pickup.velocity = { 0.0f, 0.0f };
                    pickup.settled = true;
                    break;
                }
            }
        }

        const Physics::Box playerBox = makeBox(player->getPosSafe(), player->getSizeSafe());
        if (boxesOverlap(makePickupBox(pickup), playerBox))
        {
            player->health = std::min(player->maxHealth, player->health + HealthPickupAmount);
            pickup.alive = false;
        }
    }

    mHealthPickups.erase(
        std::remove_if(mHealthPickups.begin(), mHealthPickups.end(), [](const HealthPickup& pickup) { return !pickup.alive; }),
        mHealthPickups.end()
    );
}

bool PlayState::tryStartPlayerShot()
{
    if (player == nullptr || !player->shootPressedThisFrame)
    {
        return false;
    }

    // Only three player-created shots can exist at once, including reflected
    // ones, and mashing Enter still has to wait for the cooldown.
    const bool shotLimitReached = mPlayerShots.size() >= MaxPlayerShots;
    const bool shotDelayActive = mPlayerShotCooldown > 0.0f;
    if (shotLimitReached || shotDelayActive)
    {
        player->shootPressedThisFrame = false;
        return false;
    }

    mPlayerShotCooldown = PlayerShotDelay;
    mPlayerShootPoseTimer = PlayerShootPoseSeconds;
    player->weaponIsHoldingShootPose = true;
    return true;
}

void PlayState::spawnPlayerShot()
{
    if (player == nullptr)
    {
        return;
    }

    const bool right = player->isFacingRight();
    EnergyShot shot;
    shot.radius = 7.0f;
    shot.position = player->getPosSafe() + cannonOffsetFor(*player);
    shot.position.y -= shot.radius * 2.0f;
    shot.velocity = { right ? PlayerShotSpeed : -PlayerShotSpeed, 0.0f };
    shot.fromPlayer = true;
    shot.color = sf::Color(90, 235, 255);

    mPlayerShots.emplace_back(shot);
}

void PlayState::updateEnemies(float dt)
{
    if (player == nullptr || tmap == nullptr)
    {
        return;
    }

    auto lineBlockedByTile = [this](sf::Vector2f from, sf::Vector2f to)
    {
        const sf::Vector2f delta = to - from;
        const float distance = std::sqrt((delta.x * delta.x) + (delta.y * delta.y));
        const int steps = std::max(1, static_cast<int>(distance / 32.0f));

        for (int i = 1; i < steps; ++i)
        {
            const float pct = static_cast<float>(i) / static_cast<float>(steps);
            const sf::Vector2f point = from + (delta * pct);

            for (GObj* solid : tmap->getSolids())
            {
                if (solid != nullptr && pointInsideBox(point, Physics::getBox(solid)))
                {
                    return true;
                }
            }
        }

        return false;
    };

    const sf::Vector2f target = playerCenter(*player);

    for (GuardEnemy& enemy : mEnemies)
    {
        if (!enemy.alive)
        {
            continue;
        }

        enemy.hitFlashTimer = std::max(0.0f, enemy.hitFlashTimer - dt);

        const sf::Vector2f oldSize = enemySize(enemy);
        const float feetY = enemy.position.y + oldSize.y;
        const sf::Vector2f eyes = enemyCenter(enemy);
        const sf::Vector2f toPlayer = target - eyes;

        const bool playerIsInFront = enemy.facingRight ? toPlayer.x > 0.0f : toPlayer.x < 0.0f;
        const bool playerIsClose = std::fabs(toPlayer.x) <= EnemySightDistance;
        const bool playerIsInLane = std::fabs(toPlayer.y) <= EnemySightHeight;
        const bool canSeePlayer =
            playerIsInFront &&
            playerIsClose &&
            playerIsInLane &&
            !lineBlockedByTile(eyes, target);

        // Guard mode is the crouched/shield form. While guarding the enemy
        // never turns around; the player has to leave the sight cone.
        if (enemy.guarding != canSeePlayer)
        {
            enemy.guarding = canSeePlayer;
            enemy.position.y = feetY - enemySize(enemy).y;

            if (enemy.guarding)
            {
                enemy.shotTimer = std::min(enemy.shotTimer, 0.35f);
            }
        }

        if (enemy.guarding)
        {
            // The first shot has a small tell, then every shot after that
            // follows the regular 2.5 second cadence.
            enemy.shotTimer -= dt;
            if (enemy.shotTimer <= 0.0f)
            {
                EnergyShot shot;
                shot.radius = 7.0f;
                shot.fromPlayer = false;
                shot.color = sf::Color(255, 70, 70);
                shot.position = enemyCenter(enemy);
                shot.position.x += enemy.facingRight ? enemySize(enemy).x * 0.55f : -enemySize(enemy).x * 0.55f;
                shot.position.y -= 8.0f;
                shot.velocity = normalized(target - shot.position) * EnemyShotSpeed;
                mEnemyShots.emplace_back(shot);

                enemy.shotTimer = EnemyShotDelay;
            }

            continue;
        }

        const float direction = enemy.facingRight ? 1.0f : -1.0f;
        enemy.position.x += direction * EnemyWalkSpeed * dt;

        if (enemy.position.x >= enemy.patrolRightX)
        {
            enemy.position.x = enemy.patrolRightX;
            enemy.facingRight = false;
        }
        else if (enemy.position.x <= enemy.patrolLeftX)
        {
            enemy.position.x = enemy.patrolLeftX;
            enemy.facingRight = true;
        }
    }
}

void PlayState::updatePlayerShots(float dt)
{
    if (player == nullptr || tmap == nullptr)
    {
        return;
    }

    const sf::Vector2f viewCenter = mainView.getCenter();
    const sf::Vector2f viewSize = mainView.getSize();
    const Physics::Box screenBox = makeBox(
        { viewCenter.x - (viewSize.x * 0.5f) - 96.0f, viewCenter.y - (viewSize.y * 0.5f) - 96.0f },
        { viewSize.x + 192.0f, viewSize.y + 192.0f }
    );

    for (EnergyShot& shot : mPlayerShots)
    {
        if (!shot.alive)
        {
            continue;
        }

        shot.position += shot.velocity * dt;
        Physics::Box shotBox = makeShotBox(shot);

        if (!boxesOverlap(shotBox, screenBox))
        {
            shot.alive = false;
            continue;
        }

        for (GObj* solid : tmap->getSolids())
        {
            if (solid != nullptr && boxesOverlap(shotBox, Physics::getBox(solid)))
            {
                shot.alive = false;
                break;
            }
        }

        if (!shot.alive)
        {
            continue;
        }

        if (shot.fromPlayer)
        {
            for (GuardEnemy& enemy : mEnemies)
            {
                if (!enemy.alive)
                {
                    continue;
                }

                const Physics::Box enemyBox = makeBox(enemy.position, enemySize(enemy));
                if (!boxesOverlap(shotBox, enemyBox))
                {
                    continue;
                }

                if (enemy.guarding)
                {
                    enemy.hitFlashTimer = HitFlashSeconds;

                    // Crouched enemies are shielded from the front: reflect
                    // the same player-created shot back until it hits terrain,
                    // leaves the screen, or tags the player.
                    shot.fromPlayer = false;
                    shot.velocity.x = -shot.velocity.x;
                    shot.velocity.y *= 0.25f;
                    shot.color = sf::Color(255, 230, 70);

                    if (shot.velocity.x > 0.0f)
                    {
                        shot.position.x = enemyBox.right() + shot.radius + 1.0f;
                    }
                    else
                    {
                        shot.position.x = enemyBox.left - shot.radius - 1.0f;
                    }

                    break;
                }

                enemy.hitFlashTimer = HitFlashSeconds;
                enemy.health -= 1;
                shot.alive = false;
                if (enemy.health <= 0)
                {
                    spawnHealthPickup(enemyCenter(enemy));
                    enemy.alive = false;
                }
                break;
            }
        }
        else
        {
            const Physics::Box playerBox = makeBox(player->getPosSafe(), player->getSizeSafe());
            if (boxesOverlap(shotBox, playerBox))
            {
                damagePlayer(EnemyShotDamage);
                shot.alive = false;
            }
        }
    }

    mPlayerShots.erase(
        std::remove_if(mPlayerShots.begin(), mPlayerShots.end(), [](const EnergyShot& shot) { return !shot.alive; }),
        mPlayerShots.end()
    );

    mEnemies.erase(
        std::remove_if(mEnemies.begin(), mEnemies.end(), [](const GuardEnemy& enemy) { return !enemy.alive; }),
        mEnemies.end()
    );
}

void PlayState::updateEnemyShots(float dt)
{
    if (player == nullptr || tmap == nullptr)
    {
        return;
    }

    const sf::Vector2f viewCenter = mainView.getCenter();
    const sf::Vector2f viewSize = mainView.getSize();
    const Physics::Box screenBox = makeBox(
        { viewCenter.x - (viewSize.x * 0.5f) - 96.0f, viewCenter.y - (viewSize.y * 0.5f) - 96.0f },
        { viewSize.x + 192.0f, viewSize.y + 192.0f }
    );

    for (EnergyShot& shot : mEnemyShots)
    {
        if (!shot.alive)
        {
            continue;
        }

        shot.position += shot.velocity * dt;
        const Physics::Box shotBox = makeShotBox(shot);

        if (!boxesOverlap(shotBox, screenBox))
        {
            shot.alive = false;
            continue;
        }

        for (GObj* solid : tmap->getSolids())
        {
            if (solid != nullptr && boxesOverlap(shotBox, Physics::getBox(solid)))
            {
                shot.alive = false;
                break;
            }
        }

        if (!shot.alive)
        {
            continue;
        }

        const Physics::Box playerBox = makeBox(player->getPosSafe(), player->getSizeSafe());
        if (boxesOverlap(shotBox, playerBox))
        {
            damagePlayer(EnemyShotDamage);
            shot.alive = false;
        }
    }

    mEnemyShots.erase(
        std::remove_if(mEnemyShots.begin(), mEnemyShots.end(), [](const EnergyShot& shot) { return !shot.alive; }),
        mEnemyShots.end()
    );
}

eStateID PlayState::updateImpl(float dt)
{
    if (player == nullptr || player->copy == nullptr || tmap == nullptr)
    {
        return eStateID::None;
    }

    if (mPendingState != eStateID::None && mPendingState != eStateID::Count)
    {
        eStateID tmpState = mPendingState;
        mPendingState = eStateID::None;
        return tmpState;
    }

    const bool wasGrounded = player->grounded;

    // 1. Start from the committed/live state.
    // Everything below writes to player->copy until swapdate() commits it.
    player->syncCopyFromLive();
    player->copy->grounded = false;
    player->copy->setJustLanded(false);
    player->copy->justLeftGround = false;
    clearContacts(*player->copy);

    // 2. Turn keyboard state into player intentions and copy velocity.
    handleStaticInputImpl(dt);
    updateCombatTimers(dt);

    if (player->jumpPressedThisFrame && wasGrounded)
    {
        player->copy->setVel({ player->copy->getVelSafe().x, JumpVelocity });
        player->copy->justLeftGround = true;
    }

    const bool shouldApplyGravity = !wasGrounded || player->copy->getVelSafe().y < 0.0f || player->copy->justLeftGround;
    player->copy->setAccleration({ 0.0f, shouldApplyGravity ? GravityPerFrame : 0.0f });

    // 3. Integrate velocity on the copy, then sweep that movement through tiles.
    player->updatePhysics(dt);
    resolveSweptCollisions(*player, tmap->getSolids(), dt);

    // 4. Correct any remaining overlap, then keep grounded true if the player
    // is exactly standing on a tile without moving into it this frame.
    Physics::moveFirstOutsideVector(player, tmap->getSolids(), true);
    snapToGroundIfStanding(*player, tmap->getSolids());
    player->copy->setJustLanded(!wasGrounded && player->copy->grounded);

    // 5. Choose the animation from the resolved movement state, advance the pose,
    // then commit copy -> live for rendering.
    const bool shouldSpawnPlayerShot = tryStartPlayerShot();
    const ActionIntent intent = buildPlayerIntent(*player);
    const EntityAction jumpAction = updateJumpAnimationPhase(*player, wasGrounded, dt);
    const ActionContext context = buildPlayerContext(*player, jumpAction);
    mActMgr.update(*player, intent, context);
    player->updateAnimation(dt);
    player->swapdate();

    if (shouldSpawnPlayerShot)
    {
        spawnPlayerShot();
    }

    updateEnemies(dt);
    updatePlayerShots(dt);
    updateEnemyShots(dt);
    updateHealthPickups(dt);

    return eStateID::None;
}
