#ifndef PLAYSTATE_H__
#define PLAYSTATE_H__

#include "../GameState.h"
#include <SFML/Window/Keyboard.hpp>

#include <iostream>
// A specific implementation
class PlayState : public GameState<PlayState> {
    eStateID mPendingState{ eStateID::None };

public:
    eStateID updateImpl(float dt) 
    {
        if (mPendingState != eStateID::None && mPendingState != eStateID::Count)
        {
            eStateID tmpState = mPendingState;
            mPendingState = eStateID::None;
            return tmpState;
        }
        return eStateID::None;
    }

    void renderImpl(sf::RenderWindow& window) {
    }

    void enterImpl() {
        std::cout << "Entered PlayState" << std::endl;
    }

    void leaveImpl() {
        std::cout << "Left PlayState" << std::endl;
    }

    void handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed) {

    }

};
#endif