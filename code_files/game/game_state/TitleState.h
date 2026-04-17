#ifndef TITLESTATE_H__
#define TITLESTATE_H__

#include <SFML/Window/Keyboard.hpp>
#include "../GameState.h"
#include <SFML/Graphics.hpp>
#include <iostream>
// A specific implementation
class TitleState : public GameState<TitleState> {
    eStateID mPendingState{ eStateID::None };

    Cfg::Textures mBGTexID{ Cfg::Textures::None };

    


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

        window.draw(renderSprite);

        sf::Text txt{ Cfg::fonts.get((int)Cfg::Fonts::Bubbly) };

        txt.setCharacterSize(44u);
        txt.setFillColor(sf::Color::White);
        txt.setOutlineColor(sf::Color::Black);
        txt.setString("Press Enter To Play");
        txt.setOutlineThickness(3);
        txt.setPosition({600.f, 400.f});

        window.draw(txt);



    }

    void enterImpl() {
        std::cout << "Entered TitleState" << std::endl;
        mBGTexID = Cfg::Textures::TitleBG;
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