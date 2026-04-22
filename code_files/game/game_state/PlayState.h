#ifndef PLAYSTATE_H__
#define PLAYSTATE_H__

#include "../GameState.h"
#include <SFML/Window/Keyboard.hpp>
#include <game/game_objects/AnimObj.h>
#include <iostream>
#include "../../game/map/Tilemap.h"
#include <memory>
#include <vector>
#include <string>
#include "../map/ParallaxBG.h"
#include "../io/ActionMgr.h"
#include <game_objects/Player.h>
// A specific implementation
class PlayState : public GameState<PlayState> {
public:
    struct EnergyShot
    {
        sf::Vector2f position{};
        sf::Vector2f velocity{};
        float radius{ 7.0f };
        bool fromPlayer{ true };
        bool alive{ true };
        sf::Color color{ sf::Color::Cyan };
    };

    struct GuardEnemy
    {
        sf::Vector2f position{};
        sf::Vector2f standingSize{ 64.0f, 96.0f };
        sf::Vector2f guardingSize{ 64.0f, 68.0f };
        float patrolLeftX{ 0.0f };
        float patrolRightX{ 0.0f };
        bool facingRight{ true };
        bool guarding{ false };
        bool alive{ true };
        int health{ 6 };
        float shotTimer{ 0.0f };
        float hitFlashTimer{ 0.0f };
    };

    struct HealthPickup
    {
        sf::Vector2f position{};
        sf::Vector2f velocity{};
        float radius{ 14.0f };
        bool alive{ true };
        bool settled{ false };
    };

private:
    ActionMgr mActMgr{};

    std::unique_ptr<Tilemap> tmap{};
    ParallaxBG mParallaxBG{ 8 };
    eStateID mPendingState{ eStateID::None };
    Player* player{ nullptr };
    std::vector<EnergyShot> mPlayerShots{};
    std::vector<EnergyShot> mEnemyShots{};
    std::vector<GuardEnemy> mEnemies{};
    std::vector<HealthPickup> mHealthPickups{};

    void resetCombatState();
    void spawnPlayerShot();
    void updateEnemies(float dt);
    void updatePlayerShots(float dt);
    void updateEnemyShots(float dt);
    void spawnHealthPickup(sf::Vector2f position);
    void updateHealthPickups(float dt);
    void renderCombat(sf::RenderWindow& window);
    void renderPlayerHealth(sf::RenderWindow& window);

public:
    PlayState(sf::RenderWindow& wnd);
    eStateID updateImpl(float dt);
    void renderImpl(sf::RenderWindow& window); 
    void handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed);
    void handleStaticInputImpl(float dt);
    void enterImpl();
    void leaveImpl();


};
#endif
