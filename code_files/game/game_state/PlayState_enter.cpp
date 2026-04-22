#include "PlayState.h"

void PlayState::resetCombatState()
{
    mPlayerShots.clear();
    mEnemyShots.clear();
    mEnemies.clear();
    mDamagePops.clear();
    mHealthPickups.clear();
    mPlayerShotCooldown = 0.0f;
    mPlayerShootPoseTimer = 0.0f;
    mPlayerInvincibleTimer = 0.0f;
    mPlayerHitFlashTimer = 0.0f;

    if (player != nullptr)
    {
        player->health = player->maxHealth;
        player->weaponIsHoldingShootPose = false;
    }

    GuardEnemy enemy;
    enemy.patrolLeftX = 960.0f;
    enemy.patrolRightX = 1450.0f;
    enemy.position = { 1280.0f, 832.0f - enemy.standingSize.y };
    enemy.facingRight = false;
    enemy.shotTimer = 1.0f;
    mEnemies.emplace_back(enemy);
}

void PlayState::enterImpl() {
    std::cout << "Entered PlayState" << std::endl;
    if (player)
        delete player;

    player = new Player{};
    dynamic_cast<AnimObj*>(player)->loadAnimations("assets/anims/player.anm");

    tmap = std::make_unique<Tilemap>();
    tmap->loadMap("assets/maps/tilemaps/tilemapJungle1.map", "assets/maps/tilesets/tileset_jungle1.tst");

    resetCombatState();

    // Add farthest -> closest
    mParallaxBG.addLayer(
        Cfg::textures.get((int)Cfg::Textures::PLXBG8),
        0.30f,
        0.30f,
        sf::Color(190, 200, 220, 90)     // distant, fogged out
    );

    mParallaxBG.addLayer(
        Cfg::textures.get((int)Cfg::Textures::PLXBG7),
        0.40f,
        0.40f,
        sf::Color(210, 215, 225, 140)
    );

    mParallaxBG.addLayer(
        Cfg::textures.get((int)Cfg::Textures::PLXBG6),
        0.50f,
        0.50f,
        sf::Color(235, 235, 235, 210)
    );

    mParallaxBG.addLayer(
        Cfg::textures.get((int)Cfg::Textures::PLXBG5),
        0.60f,
        0.60f,
        sf::Color::White
    );
    mParallaxBG.addLayer(
        Cfg::textures.get((int)Cfg::Textures::PLXBG4),
        0.70f,
        0.70f,
        sf::Color(190, 200, 220, 90)     // distant, fogged out
    );

    mParallaxBG.addLayer(
        Cfg::textures.get((int)Cfg::Textures::PLXBG3),
        0.80f,
        0.80f,
        sf::Color(210, 215, 225, 140)
    );

    mParallaxBG.addLayer(
        Cfg::textures.get((int)Cfg::Textures::PLXBG2),
        0.90f,
        0.90f,
        sf::Color(235, 235, 235, 210)
    );

    mParallaxBG.addLayer(
        Cfg::textures.get((int)Cfg::Textures::PLXBG1),
        1.00f,
        1.00f,
        sf::Color::White
    );

    mParallaxBG.setAnchorWorldPosition(0.0f, 0.0f);
}
