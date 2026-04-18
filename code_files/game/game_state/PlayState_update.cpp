#include "PlayState.h"

eStateID PlayState::updateImpl(float dt)
{
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