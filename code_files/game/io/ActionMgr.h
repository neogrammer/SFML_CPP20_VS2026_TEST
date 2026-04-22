#pragma once

#include "../game_objects/AnimObj.h"

#include <cmath>

enum class EntityAction
{
    None = 0,
    TeleportIn,
    TeleportLand,
    Idle,
    Run,
    LiftOff,
    Rising,
    JumpPeak,
    Falling,
    Landing,
    DashStart,
    Dashing,
    DashEnd,
    WallKick,
    WallLand,
    WallSlide
};

class ActionIntent
{
public:
    float moveX{ 0.f };     // -1..1
    float moveY{ 0.f };     // for ladder / vertical intent if you need it later

    bool jumpPressed{ false };
    bool jumpHeld{ false };

    bool dashPressed{ false };
    bool dashHeld{ false };

    bool shootPressed{ false };
    bool shootHeld{ false };

    bool teleportPressed{ false };
};

class ActionContext
{
public:
    EntityAction forcedAction{ EntityAction::None };
    sf::Vector2f velocity{ 0.f, 0.f };

    bool grounded{ false };
    bool justLeftGround{ false };
    bool justLanded{ false };

    bool dashStarting{ false };
    bool dashing{ false };
    bool dashEnding{ false };

    bool wallKicking{ false };
    bool wallLanding{ false };
    bool wallSliding{ false };

    bool teleportingIn{ false };
    bool teleportLanding{ false };

    bool shootingLocked{ false };   // enemy script or weapon system can force shooting pose
};

class ActionMgr
{
public:
    void setRunThreshold(float value);
    void setJumpPeakThreshold(float value);
    void setInputDeadzone(float value);

    void update(AnimObj& obj, const ActionIntent& intent, const ActionContext& context);

    EntityAction getCurrentAction() const;
    AnimName getCurrentDesiredAnim() const;

private:
    void updateFacing(AnimObj& obj, const ActionIntent& intent);
    EntityAction resolveAction(const ActionIntent& intent, const ActionContext& context) const;
    AnimName resolveAnimation(EntityAction action, bool wantsShoot) const;

    AnimName toShootVariant(EntityAction action) const;
    AnimName toBaseVariant(EntityAction action) const;

private:
    EntityAction m_currentAction{ EntityAction::None };
    AnimName m_currentDesiredAnim{ AnimName::None };

    float m_runThreshold{ 0.10f };
    float m_jumpPeakThreshold{ 15.0f };
    float m_inputDeadzone{ 0.15f };
};
