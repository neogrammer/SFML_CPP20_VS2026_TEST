#include "PlayState.h"

eStateID PlayState::updateImpl(float dt)
{
    bool d = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    bool a = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    auto& c = *dynamic_cast<AnimObj*>(gameObject);
    if (a || d)
    {
        if (a && d)
        {

        }
        else
        {
            if (d)
            {
                gameObject->setVel({ 500.f, gameObject->getVel().y });
                gameObject->setFacingRight(true);
                if (c.getCurrentAnim() != AnimName::Run)
                {
                    c.setCurrentAnim(AnimName::Run);
                }

            }
            else
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