#include "PlayState.h"

namespace
{
    constexpr float RunVelocity = 500.0f;
}

void PlayState::handleStaticInputImpl(float dt)
{
    (void)dt;

    if (player == nullptr || player->copy == nullptr)
    {
        return;
    }

    const bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    const bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    const bool jump = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    const bool shoot = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);

    // Store raw intent on the player. Movement and animation both read these
    // same flags, so controls, physics, and pose selection stay in agreement.
    player->rightHeld = right;
    player->leftHeld = left;

    player->jumpPressedThisFrame = jump && !player->jumpHeld;
    player->jumpHeld = jump;
    player->shootPressedThisFrame = shoot && !player->shootHeld;
    player->shootHeld = shoot;

    sf::Vector2f velocity = player->copy->getVelSafe();

    if (right == left)
    {
        velocity.x = 0.0f;
    }
    else if (right)
    {
        velocity.x = RunVelocity;
        player->setFacingRightCpy(true);
    }
    else
    {
        velocity.x = -RunVelocity;
        player->setFacingRightCpy(false);
    }

    player->copy->setVel(velocity);
}
