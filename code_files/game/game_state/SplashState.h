#ifndef SPLASHSTATE_H__
#define SPLASHSTATE_H__

#include "../GameState.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
// A specific implementation
class SplashState : public GameState<SplashState> {
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
        std::cout << "Entered SplashState" << std::endl;
    }

    void leaveImpl() {
        std::cout << "Left SplashState" << std::endl;
    }

    void handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed) {
        if (key == sf::Keyboard::Key::Enter && !isPressed) {
            // Move player up...
            mPendingState = eStateID::Title;
        }
    }
};

#endif