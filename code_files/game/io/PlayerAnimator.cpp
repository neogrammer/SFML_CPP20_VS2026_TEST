#include "PlayerAnimator.h"
#include <game_objects/Player.h>

void PlayerAnimator::evalContext()
{}

AnimName PlayerAnimator::getNextAnimAndClear()
{
    return AnimName::Idle;
}

void PlayerAnimator::switchAnimation(AnimName anim)
{}
