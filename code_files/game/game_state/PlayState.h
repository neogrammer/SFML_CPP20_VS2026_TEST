#ifndef PLAYSTATE_H__
#define PLAYSTATE_H__

#include "../GameState.h"
#include <SFML/Window/Keyboard.hpp>
#include <game/game_objects/AnimObj.h>
#include <game/game_objects/CombatEntities.h>
#include <iostream>
#include "../../game/map/Tilemap.h"
#include <memory>
#include <vector>
#include <string>
#include "../map/ParallaxBG.h"
#include <game_objects/Player.h>
// A specific implementation
class PlayState : public GameState<PlayState> {
private:
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
    void prepareSimulationFrame();
    void updateGameplayEntities(float dt);
    void resolveGameplayCollisions();
    void swapGameplayData();
    void pruneDestroyedEntities();
    void updateCameraForPlayer();
    void renderWorldEntities(sf::RenderWindow& window);
    void renderPlayer(sf::RenderWindow& window);
    void renderForegroundEffects(sf::RenderWindow& window);
    void renderGameplayUI(sf::RenderWindow& window);
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
