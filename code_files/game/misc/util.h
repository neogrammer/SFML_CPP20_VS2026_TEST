#ifndef UTIL_H__
#define UTIL_H__
#include <SFML/Graphics.hpp>

namespace util{

    static void centerTextV(sf::Text& txt_, uint32_t scrH_)
    {
        sf::FloatRect textBounds = txt_.getLocalBounds();

        // Set origin to the center of the text
        txt_.setOrigin(
            { txt_.getOrigin().x,
            textBounds.position.y + textBounds.size.y / 2.0f }
        );

        // Position text at the center of the window
        txt_.setPosition({ txt_.getPosition().x, (float)scrH_ / 2.0f });
    }


    static void centerTextH(sf::Text& txt_, uint32_t scrW_)
    {

        sf::FloatRect textBounds = txt_.getLocalBounds();

        // Set origin to the center of the text
        txt_.setOrigin(
            { textBounds.position.x + textBounds.size.x / 2.0f,
            txt_.getOrigin().y }
        );

        // Position text at the center of the window
        txt_.setPosition({ (float)scrW_ / 2.0f, txt_.getPosition().y });
    }


	static void centerText(sf::Text& txt_, const sf::Vector2u& screenSize_) {
        sf::FloatRect textBounds = txt_.getLocalBounds();

        // Set origin to the center of the text
        txt_.setOrigin(
            { textBounds.position.x + textBounds.size.x / 2.0f,
            textBounds.position.y + textBounds.size.y / 2.0f }
        );

        // Position text at the center of the window
        txt_.setPosition({ screenSize_.x / 2.0f, screenSize_.y / 2.0f });

	}

}

#endif