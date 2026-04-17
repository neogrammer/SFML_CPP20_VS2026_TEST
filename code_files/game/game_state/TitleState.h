#ifndef TITLESTATE_H__
#define TITLESTATE_H__

#include <SFML/Window/Keyboard.hpp>
#include "../GameState.h"
#include <iostream>
// A specific implementation
class TitleState : public GameState<TitleState> {
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
        std::cout << "Entered TitleState" << std::endl;

    }

    void leaveImpl() {
        std::cout << "Left TitleState" << std::endl;

    }

    void handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed) {
        if (key == sf::Keyboard::Key::Enter && !isPressed) {
            // Move player up...
            mPendingState = eStateID::Play;
        }
    }

};

#endif