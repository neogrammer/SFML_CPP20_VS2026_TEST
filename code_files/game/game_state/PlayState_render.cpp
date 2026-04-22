#include "PlayState.h"

#include <cstdint>
#include <string>

namespace
{
    sf::Vector2f renderedEnemySize(const PlayState::GuardEnemy& enemy)
    {
        return enemy.guarding ? enemy.guardingSize : enemy.standingSize;
    }
}

void PlayState::renderCombat(sf::RenderWindow& window)
{
    for (const GuardEnemy& enemy : mEnemies)
    {
        if (!enemy.alive)
        {
            continue;
        }

        sf::RectangleShape body;
        body.setPosition(enemy.position);
        body.setSize(renderedEnemySize(enemy));
        const bool flashing = enemy.hitFlashTimer > 0.0f;
        body.setFillColor(flashing ? sf::Color::White : (enemy.guarding ? sf::Color(180, 20, 20) : sf::Color(235, 35, 35)));
        body.setOutlineColor(flashing ? sf::Color::White : (enemy.guarding ? sf::Color(255, 230, 60) : sf::Color(70, 0, 0)));
        body.setOutlineThickness(3.0f);
        window.draw(body);

        sf::RectangleShape eye;
        eye.setSize({ 9.0f, 9.0f });
        eye.setFillColor(sf::Color::White);
        eye.setPosition({
            enemy.position.x + (enemy.facingRight ? renderedEnemySize(enemy).x - 18.0f : 9.0f),
            enemy.position.y + 18.0f
        });
        window.draw(eye);
    }

    for (const HealthPickup& pickup : mHealthPickups)
    {
        if (!pickup.alive)
        {
            continue;
        }

        sf::CircleShape orb{ pickup.radius, 28 };
        orb.setPosition({ pickup.position.x - pickup.radius, pickup.position.y - pickup.radius });
        orb.setFillColor(sf::Color(255, 220, 40));
        orb.setOutlineColor(sf::Color(255, 255, 210));
        orb.setOutlineThickness(3.0f);
        window.draw(orb);

        sf::CircleShape shine{ pickup.radius * 0.35f, 16 };
        shine.setPosition({
            pickup.position.x - (pickup.radius * 0.55f),
            pickup.position.y - (pickup.radius * 0.65f)
        });
        shine.setFillColor(sf::Color(255, 255, 255, 170));
        window.draw(shine);
    }

    auto drawShot = [&window](const EnergyShot& shot)
    {
        sf::CircleShape circle{ shot.radius, 20 };
        circle.setPosition({ shot.position.x - shot.radius, shot.position.y - shot.radius });
        circle.setFillColor(shot.color);
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(1.0f);
        window.draw(circle);
    };

    for (const EnergyShot& shot : mPlayerShots)
    {
        drawShot(shot);
    }

    for (const EnergyShot& shot : mEnemyShots)
    {
        drawShot(shot);
    }
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

    for (const DamagePop& pop : mDamagePops)
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

    const bool playerBlinkOff =
        mPlayerInvincibleTimer > 0.0f &&
        mPlayerHitFlashTimer <= 0.0f &&
        (static_cast<int>(mPlayerInvincibleTimer / 0.08f) % 2 == 0);

    if (!playerBlinkOff)
    {
        window.draw(*player->sprite());
    }

    if (mPlayerHitFlashTimer > 0.0f)
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

    renderCombat(window);

    window.setView(window.getDefaultView());
    renderPlayerHealth(window);
}
