#include "PlayState.h"

void PlayState::renderImpl(sf::RenderWindow& window)
{
    if (player == nullptr || tmap == nullptr)
    {
        return;
    }

    const sf::Vector2f playerCenter{
        player->getPosSafe().x + (player->getSizeSafe().x * 0.5f),
        player->getPosSafe().y + (player->getSizeSafe().y * 0.5f)
    };

    // The camera only advances to the right. Once the player crosses the
    // current view center, the view locks its x center to the player center.
    sf::Vector2f viewCenter = mainView.getCenter();
    if (playerCenter.x > viewCenter.x)
    {
        viewCenter.x = playerCenter.x;
        mainView.setCenter(viewCenter);
        mParallaxBG.update(mainView);
    }

    window.setView(mainView);

    window.draw(mParallaxBG);
    tmap->renderMap(window);
    window.draw(*player->sprite());

    sf::RectangleShape playerBounds;
    playerBounds.setFillColor(sf::Color::Transparent);
    playerBounds.setOutlineColor(sf::Color(255, 0, 0, 190));
    playerBounds.setOutlineThickness(2.0f);
    playerBounds.setPosition(player->getPosSafe());
    playerBounds.setSize(player->getSizeSafe());

    window.draw(playerBounds);
}
