#include "PlayState.h"

void PlayState::handleStaticInputImpl(float dt, GObj* gameObject)
{
    constexpr float bufferTime = { 0.03f };
    static float bufferedLeft = 0.f;
    static float bufferedRight = 0.f;
    bool d = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    bool a = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);

    if (d)
    {
        bufferedRight += dt;
    }
    else
    {
        bufferedRight = 0.f;
    }

    if (a)
    {
        bufferedLeft += dt;
    }
    else
    {
        bufferedLeft = 0.f;
    }

    auto& c = *dynamic_cast<AnimObj*>(gameObject);
    if (a || d)
    {
        if (a && d)
        {
            if (gameObject->getVelocity().x < -0.001f)
                gameObject->setVel({ -500.f, gameObject->getVel().y });
            else if (gameObject->getVelocity().x > 0.001f)
                gameObject->setVel({ 500.f, gameObject->getVel().y });


            if (gameObject->getVelocity().x >= -0.001f && gameObject->getVelocity().x <= 0.001f)
            {
                if (c.getCurrentAnim() != AnimName::Idle)
                {
                    c.setCurrentAnim(AnimName::Idle);
                }
            }
            else
            {
                if (c.getCurrentAnim() != AnimName::Run)
                {
                    c.setCurrentAnim(AnimName::Run);
                }
            }

        }
        else
        {
            if (d && bufferedRight >= bufferTime)
            {
                gameObject->setVel({ 500.f, gameObject->getVel().y });
                gameObject->setFacingRightCpy(true);
                if (c.getCurrentAnim() != AnimName::Run)
                {
                    c.setCurrentAnim(AnimName::Run);
                }

            }
            if (a && bufferedLeft >= bufferTime)
            {

                gameObject->setVel({ -500.f, gameObject->getVel().y });
                gameObject->setFacingRightCpy(false);
                if (c.getCurrentAnim() != AnimName::Run)
                {
                    c.setCurrentAnim(AnimName::Run);
                }

            }
        }
    }
    else
    {
        if ((!d && !a))
        {
            gameObject->setVel({ 0.f, gameObject->getVel().y });

            if (c.getCurrentAnim() != AnimName::Idle)
            {
                c.setCurrentAnim(AnimName::Idle);
            }
        }
    }
}