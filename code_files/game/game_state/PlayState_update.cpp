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
        intent.shootHeld = player.shootHeld;

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
}

PlayState::PlayState(sf::RenderWindow& wnd)
    : GameState<PlayState>{}
{
    mainView = wnd.getDefaultView();
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
    const ActionIntent intent = buildPlayerIntent(*player);
    const EntityAction jumpAction = updateJumpAnimationPhase(*player, wasGrounded, dt);
    const ActionContext context = buildPlayerContext(*player, jumpAction);
    mActMgr.update(*player, intent, context);
    player->updateAnimation(dt);
    player->swapdate();

    return eStateID::None;
}
