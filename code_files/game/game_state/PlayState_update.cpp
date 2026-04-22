#include "PlayState.h"

#include <algorithm>

namespace
{
    constexpr int MaxPlayerShots = 3;
}

PlayState::PlayState(sf::RenderWindow& wnd)
    : GameState<PlayState>{}
{
    mainView = wnd.getDefaultView();
}

eStateID PlayState::updateImpl(float dt)
{
    if (player == nullptr || player->copy == nullptr || tmap == nullptr)
    {
        return eStateID::None;
    }

    if (mPendingState != eStateID::None && mPendingState != eStateID::Count)
    {
        eStateID tmpState = mPendingState;
        mPendingState = eStateID::None;
        return tmpState;
    }

    prepareSimulationFrame();
    handleStaticInputImpl(dt);

    updateGameplayEntities(dt);
    resolveGameplayCollisions();
    swapGameplayData();
    pruneDestroyedEntities();

    return eStateID::None;
}



void PlayState::spawnPlayerShot()
{
    if (player == nullptr)
    {
        return;
    }

    mPlayerShots.emplace_back(EnergyShot::makePlayerShot(*player));
}

void PlayState::prepareSimulationFrame()
{
    player->beginFrame();

    for (GuardEnemy& enemy : mEnemies)
    {
        enemy.syncCopyFromLive();
    }

    for (EnergyShot& shot : mPlayerShots)
    {
        shot.syncCopyFromLive();
    }

    for (EnergyShot& shot : mEnemyShots)
    {
        shot.syncCopyFromLive();
    }

    for (HealthPickup& pickup : mHealthPickups)
    {
        pickup.syncCopyFromLive();
    }
}

void PlayState::updateGameplayEntities(float dt)
{
    player->update(dt, mPlayerShots.size(), MaxPlayerShots);

    for (GuardEnemy& enemy : mEnemies)
    {
        enemy.update(dt, *player, mEnemyShots);
    }

    for (EnergyShot& shot : mPlayerShots)
    {
        shot.update(dt, mainView);
    }

    for (EnergyShot& shot : mEnemyShots)
    {
        shot.update(dt, mainView);
    }

    for (HealthPickup& pickup : mHealthPickups)
    {
        pickup.update(dt);
    }
}

void PlayState::resolveGameplayCollisions()
{
    auto playerCollisions = player->CheckAndStoreCollisions(*tmap, mEnemies, mHealthPickups);
    player->ResolveClosestCollisionsFirst(playerCollisions);

    for (GuardEnemy& enemy : mEnemies)
    {
        auto collisions = enemy.CheckAndStoreCollisions(*tmap, mPlayerShots);
        enemy.ResolveClosestCollisionsFirst(collisions);
    }

    for (EnergyShot& shot : mPlayerShots)
    {
        auto collisions = shot.CheckAndStoreCollisions(*tmap, mainView, *player, mEnemies);
        shot.ResolveClosestCollisionsFirst(collisions, *player, mEnemies, mHealthPickups);
    }

    for (EnergyShot& shot : mEnemyShots)
    {
        auto collisions = shot.CheckAndStoreCollisions(*tmap, mainView, *player, mEnemies);
        shot.ResolveClosestCollisionsFirst(collisions, *player, mEnemies, mHealthPickups);
    }

    for (HealthPickup& pickup : mHealthPickups)
    {
        auto collisions = pickup.CheckAndStoreCollisions(*tmap);
        pickup.ResolveClosestCollisionsFirst(collisions);
    }
}

void PlayState::swapGameplayData()
{
    player->SwapData();

    if (player->consumeShotRequest())
    {
        spawnPlayerShot();
    }

    for (GuardEnemy& enemy : mEnemies)
    {
        enemy.SwapData();
    }

    for (EnergyShot& shot : mPlayerShots)
    {
        shot.SwapData();
    }

    for (EnergyShot& shot : mEnemyShots)
    {
        shot.SwapData();
    }

    for (HealthPickup& pickup : mHealthPickups)
    {
        pickup.SwapData();
    }
}

void PlayState::pruneDestroyedEntities()
{
    mPlayerShots.erase(
        std::remove_if(mPlayerShots.begin(), mPlayerShots.end(), [](const EnergyShot& shot) { return !shot.isAlive(); }),
        mPlayerShots.end()
    );

    mEnemyShots.erase(
        std::remove_if(mEnemyShots.begin(), mEnemyShots.end(), [](const EnergyShot& shot) { return !shot.isAlive(); }),
        mEnemyShots.end()
    );

    mEnemies.erase(
        std::remove_if(mEnemies.begin(), mEnemies.end(), [](const GuardEnemy& enemy) { return !enemy.isAlive(); }),
        mEnemies.end()
    );

    mHealthPickups.erase(
        std::remove_if(mHealthPickups.begin(), mHealthPickups.end(), [](const HealthPickup& pickup) { return !pickup.isAlive(); }),
        mHealthPickups.end()
    );
}

