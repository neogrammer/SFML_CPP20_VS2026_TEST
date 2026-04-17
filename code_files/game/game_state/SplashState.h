#ifndef SPLASHSTATE_H__
#define SPLASHSTATE_H__

#include "../GameState.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

#include <res/Cfg.h>

// A specific implementation
class SplashState : public GameState<SplashState> {
    eStateID mPendingState{ eStateID::None };

    Cfg::Textures mBGTexID{ Cfg::Textures::SplashBG };
    Cfg::Textures mBGTexBallsID{ Cfg::Textures::BallsBG };

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

    void renderImpl(sf::RenderWindow& window) 
    {
        auto& tex = Cfg::textures.get((int)mBGTexID);

        sf::Sprite renderSprite{ tex };
        auto& tex2 = Cfg::textures.get((int)mBGTexBallsID);
        
        sf::Sprite renderSprite2{ tex2 };
        renderSprite2.setPosition({-50.f, 0.f});
        window.draw(renderSprite);
        window.draw(renderSprite2);

    }

    void enterImpl()
    {
        std::cout << "Entered SplashState" << std::endl;
    }

    void leaveImpl() 
    {
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