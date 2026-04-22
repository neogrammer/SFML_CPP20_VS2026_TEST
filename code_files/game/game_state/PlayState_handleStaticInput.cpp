#include "PlayState.h"

void PlayState::handleStaticInputImpl(float dt)
{
    if (player == nullptr)
    {
        return;
    }

    player->handleInput();

    if (tmap == nullptr)
    {
        return;
    }

    for (GuardEnemy& enemy : mEnemies)
    {
        enemy.handleScriptInput(*player, *tmap);
    }

    for (EnergyShot& shot : mPlayerShots)
    {
        shot.handleScriptInput(dt);
    }

    for (EnergyShot& shot : mEnemyShots)
    {
        shot.handleScriptInput(dt);
    }

    for (HealthPickup& pickup : mHealthPickups)
    {
        pickup.handleScriptInput(dt);
    }
}
