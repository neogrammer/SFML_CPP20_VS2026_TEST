#include "PlayState.h"

#include <cstdint>
#include <string>

namespace
{
    template <typename Entity>
    void drawEntityContainer(sf::RenderWindow& window, const std::vector<Entity>& entities)
    {
        for (const Entity& entity : entities)
        {
            entity.render(window);
        }
    }
}

void PlayState::updateCameraForPlayer()
{
    const sf::Vector2f playerCenter{
        player->getPosSafe().x + (player->getSizeSafe().x * 0.5f),
        player->getPosSafe().y + (player->getSizeSafe().y * 0.5f)
    };

    sf::Vector2f viewCenter = mainView.getCenter();
    if (playerCenter.x > viewCenter.x)
    {
        viewCenter.x = playerCenter.x;
        mainView.setCenter(viewCenter);
        mParallaxBG.update(mainView);
    }
}

void PlayState::renderWorldEntities(sf::RenderWindow& window)
{
    drawEntityContainer(window, mEnemies);
    drawEntityContainer(window, mHealthPickups);
    drawEntityContainer(window, mPlayerShots);
    drawEntityContainer(window, mEnemyShots);
}

void PlayState::renderPlayer(sf::RenderWindow& window)
{
    if (!player->shouldBlinkOff())
    {
        window.draw(*player->sprite());
    }
}

void PlayState::renderForegroundEffects(sf::RenderWindow& window)
{
    if (player->isHitFlashActive())
    {
        sf::RectangleShape playerFlash;
        playerFlash.setPosition(player->getPosSafe());
        playerFlash.setSize(player->getSizeSafe());
        playerFlash.setFillColor(sf::Color(255, 255, 255, 210));
        window.draw(playerFlash);
    }

    sf::RectangleShape playerBounds;
    playerBounds.setFillColor(sf::Color::Transparent);
    playerBounds.setOutlineColor(sf::Color(255, 0, 0, 190));
    playerBounds.setOutlineThickness(2.0f);
    playerBounds.setPosition(player->getPosSafe());
    playerBounds.setSize(player->getSizeSafe());
    window.draw(playerBounds);
}

void PlayState::renderGameplayUI(sf::RenderWindow& window)
{
    window.setView(window.getDefaultView());
    renderPlayerHealth(window);
}

void PlayState::renderPlayerHealth(sf::RenderWindow& window)
{
    if (player == nullptr)
    {
        return;
    }

    constexpr float SegmentWidth = 12.0f;
    constexpr float SegmentHeight = 18.0f;
    constexpr float Gap = 2.0f;
    const sf::Vector2f start{ 22.0f, 22.0f };

    for (int i = 0; i < player->maxHealth; ++i)
    {
        sf::RectangleShape segment;
        segment.setSize({ SegmentWidth, SegmentHeight });
        segment.setPosition({ start.x + (i * (SegmentWidth + Gap)), start.y });
        segment.setFillColor(i < player->health ? sf::Color(80, 230, 110) : sf::Color(40, 45, 48));
        segment.setOutlineColor(sf::Color(10, 16, 20));
        segment.setOutlineThickness(1.0f);
        window.draw(segment);
    }

    for (const Player::DamagePop& pop : player->getDamagePops())
    {
        float fade = pop.timer / pop.lifetime;
        if (fade < 0.0f)
        {
            fade = 0.0f;
        }
        if (fade > 1.0f)
        {
            fade = 1.0f;
        }

        sf::Text text{ Cfg::fonts.get((int)Cfg::Fonts::Bubbly) };
        text.setString("-" + std::to_string(pop.amount));
        text.setCharacterSize(24u);
        text.setFillColor(sf::Color(255, 70, 70, static_cast<std::uint8_t>(255.0f * fade)));
        text.setOutlineColor(sf::Color(20, 0, 0, static_cast<std::uint8_t>(220.0f * fade)));
        text.setOutlineThickness(2.0f);
        text.setPosition(pop.position);
        window.draw(text);
    }
}

void PlayState::renderImpl(sf::RenderWindow& window)
{
    if (player == nullptr || tmap == nullptr)
    {
        return;
    }

    updateCameraForPlayer();

    window.setView(mainView);
    window.draw(mParallaxBG);
    tmap->renderScreen(window, mainView);

    renderWorldEntities(window);
    renderPlayer(window);
    renderForegroundEffects(window);
    renderGameplayUI(window);
}
