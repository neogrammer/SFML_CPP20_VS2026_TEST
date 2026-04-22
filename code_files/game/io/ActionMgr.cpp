#include "ActionMgr.h"

void ActionMgr::setRunThreshold(float value)
{
    m_runThreshold = value;
}

void ActionMgr::setJumpPeakThreshold(float value)
{
    m_jumpPeakThreshold = value;
}

void ActionMgr::setInputDeadzone(float value)
{
    m_inputDeadzone = value;
}

void ActionMgr::update(AnimObj& obj, const ActionIntent& intent, const ActionContext& context)
{
    updateFacing(obj, intent);

    const EntityAction resolvedAction = resolveAction(intent, context);
    const bool wantsShoot = intent.shootHeld || intent.shootPressed || context.shootingLocked;
    const AnimName resolvedAnim = resolveAnimation(resolvedAction, wantsShoot);

    m_currentAction = resolvedAction;
    m_currentDesiredAnim = resolvedAnim;

    if (resolvedAnim != AnimName::None && obj.getCurrentAnim() != resolvedAnim)
    {
        obj.setCurrentAnim(resolvedAnim);
    }
}

EntityAction ActionMgr::getCurrentAction() const
{
    return m_currentAction;
}

AnimName ActionMgr::getCurrentDesiredAnim() const
{
    return m_currentDesiredAnim;
}

void ActionMgr::updateFacing(AnimObj& obj, const ActionIntent& intent)
{
    if (intent.moveX > m_inputDeadzone)
    {
        obj.setFacingRightCpy(true);
    }
    else if (intent.moveX < -m_inputDeadzone)
    {
        obj.setFacingRightCpy(false);
    }
}

EntityAction ActionMgr::resolveAction(const ActionIntent& intent, const ActionContext& context) const
{
    if (context.teleportingIn)
    {
        return EntityAction::TeleportIn;
    }

    if (context.teleportLanding)
    {
        return EntityAction::TeleportLand;
    }

    if (context.dashStarting)
    {
        return EntityAction::DashStart;
    }

    if (context.dashing)
    {
        return EntityAction::Dashing;
    }

    if (context.dashEnding)
    {
        return EntityAction::DashEnd;
    }

    if (context.wallKicking)
    {
        return EntityAction::WallKick;
    }

    if (context.wallLanding)
    {
        return EntityAction::WallLand;
    }

    if (context.wallSliding)
    {
        return EntityAction::WallSlide;
    }

    if (context.forcedAction != EntityAction::None)
    {
        return context.forcedAction;
    }

    if (context.justLanded)
    {
        return EntityAction::Landing;
    }

    if (!context.grounded)
    {
        if (context.justLeftGround&& context.velocity.y < 0.f)
        {
            return EntityAction::LiftOff;
        }

        if (std::fabs(context.velocity.y) <= m_jumpPeakThreshold)
        {
            return EntityAction::JumpPeak;
        }

        if (context.velocity.y < 0.f)
        {
            return EntityAction::Rising;
        }

        return EntityAction::Falling;
    }

    const bool wantsRun =
        std::fabs(intent.moveX) > m_inputDeadzone ||
        std::fabs(context.velocity.x) > m_runThreshold;

    if (wantsRun)
    {
        return EntityAction::Run;
    }

    return EntityAction::Idle;
}

AnimName ActionMgr::resolveAnimation(EntityAction action, bool wantsShoot) const
{
    if (wantsShoot)
    {
        return toShootVariant(action);
    }

    return toBaseVariant(action);
}

AnimName ActionMgr::toBaseVariant(EntityAction action) const
{
    switch (action)
    {
    case EntityAction::TeleportIn:   return AnimName::TeleportIn;
    case EntityAction::TeleportLand: return AnimName::TeleportLand;
    case EntityAction::Idle:         return AnimName::Idle;
    case EntityAction::Run:          return AnimName::Run;
    case EntityAction::LiftOff:      return AnimName::LiftOff;
    case EntityAction::Rising:       return AnimName::Rising;
    case EntityAction::JumpPeak:     return AnimName::JumpPeak;
    case EntityAction::Falling:      return AnimName::Falling;
    case EntityAction::Landing:      return AnimName::Landing;
    case EntityAction::DashStart:    return AnimName::DashStart;
    case EntityAction::Dashing:      return AnimName::Dashing;
    case EntityAction::DashEnd:      return AnimName::DashEnd;
    case EntityAction::WallKick:     return AnimName::WallKick;
    case EntityAction::WallLand:     return AnimName::WallLand;
    case EntityAction::WallSlide:    return AnimName::WallSlide;
    default:                         return AnimName::Idle;
    }
}

AnimName ActionMgr::toShootVariant(EntityAction action) const
{
    switch (action)
    {
    case EntityAction::Idle:         return AnimName::Shoot;
    case EntityAction::Run:          return AnimName::RunShoot;
    case EntityAction::LiftOff:      return AnimName::LiftOffShoot;
    case EntityAction::Rising:       return AnimName::RisingShoot;
    case EntityAction::JumpPeak:     return AnimName::JumpPeakShoot;
    case EntityAction::Falling:      return AnimName::FallingShoot;
    case EntityAction::Landing:      return AnimName::LandingShoot;
    case EntityAction::DashStart:    return AnimName::DashStartShoot;
    case EntityAction::Dashing:      return AnimName::DashingShoot;
    case EntityAction::DashEnd:      return AnimName::DashEndShoot;
    case EntityAction::WallKick:     return AnimName::WallKickShoot;
    case EntityAction::WallLand:     return AnimName::WallLandShoot;
    case EntityAction::WallSlide:    return AnimName::WallSlideShoot;

        // no special shoot versions for these in your enum; fall back
    case EntityAction::TeleportIn:   return AnimName::TeleportIn;
    case EntityAction::TeleportLand: return AnimName::TeleportLand;

    default:                         return AnimName::Shoot;
    }
}
