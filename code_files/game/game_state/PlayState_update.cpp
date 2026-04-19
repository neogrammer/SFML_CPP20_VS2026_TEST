#include "PlayState.h"

eStateID PlayState::updateImpl(float dt)
{
    constexpr float bufferTime = {0.03f};
    static float bufferedLeft=0.f;
    static float bufferedRight=0.f;
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
                gameObject->setFacingRight(true);
                if (c.getCurrentAnim() != AnimName::Run)
                {
                    c.setCurrentAnim(AnimName::Run);
                }

            }
            if (a && bufferedLeft >= bufferTime)
            {

                gameObject->setVel({ -500.f, gameObject->getVel().y });
                gameObject->setFacingRight(false);
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


    


    if (mPendingState != eStateID::None && mPendingState != eStateID::Count)
    {
        eStateID tmpState = mPendingState;
        mPendingState = eStateID::None;
        return tmpState;
    }

    // update back buffer for each entity first
    gameObject->update(dt);


    // after all the objects are updated in the back buffer, swap the buffer with the frontbuffer
    gameObject->swapdate();

    return eStateID::None;
    
}