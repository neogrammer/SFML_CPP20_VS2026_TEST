#pragma once

#include "../Physics.h"

#include <SFML/Graphics.hpp>
#include <vector>

class Player;
class Tilemap;

class HealthPickup;
class GuardEnemy;

class EnergyShot
{
public:
    enum class CollisionType
    {
        ScreenExit,
        Tile,
        Player,
        Enemy
    };

    struct Collision
    {
        CollisionType type{ CollisionType::Tile };
        float distanceSq{ 0.0f };
        GObj* tile{ nullptr };
        Player* player{ nullptr };
        GuardEnemy* enemy{ nullptr };
    };

    EnergyShot();

    static EnergyShot makePlayerShot(Player& player);
    static EnergyShot makeEnemyShot(sf::Vector2f position, sf::Vector2f target);

    void syncCopyFromLive();
    void handleScriptInput(float dt);
    void update(float dt, const sf::View& view);

    std::vector<Collision> CheckAndStoreCollisions(
        Tilemap& tilemap,
        const sf::View& view,
        Player& player,
        std::vector<GuardEnemy>& enemies
    );

    void ResolveClosestCollisionsFirst(
        std::vector<Collision>& collisions,
        Player& player,
        std::vector<GuardEnemy>& enemies,
        std::vector<HealthPickup>& healthPickups
    );

    void SwapData();
    void render(sf::RenderWindow& window) const;

    bool isAlive() const;
    bool willBeAlive() const;
    bool isFromPlayer() const;
    Physics::Box getCollisionBox() const;

private:
    struct State
    {
        sf::Vector2f position{};
        sf::Vector2f velocity{};
        float radius{ 7.0f };
        bool fromPlayer{ true };
        bool canDamagePlayer{ false };
        bool alive{ true };
        sf::Color color{ sf::Color::Cyan };
    };

    State mLive{};
    State mNext{};
};

class GuardEnemy
{
public:
    struct Collision
    {
        float distanceSq{ 0.0f };
    };

    GuardEnemy();

    static GuardEnemy makePatroller(
        sf::Vector2f position,
        float patrolLeftX,
        float patrolRightX,
        bool facingRight,
        float firstShotDelay
    );

    void syncCopyFromLive();
    void handleScriptInput(Player& player, Tilemap& tilemap);
    void update(float dt, Player& player, std::vector<EnergyShot>& enemyShots);

    std::vector<Collision> CheckAndStoreCollisions(Tilemap& tilemap, std::vector<EnergyShot>& playerShots);
    void ResolveClosestCollisionsFirst(std::vector<Collision>& collisions);

    void SwapData();
    void render(sf::RenderWindow& window) const;

    bool isAlive() const;
    bool willBeAlive() const;
    bool isGuarding() const;
    bool isFacingRight() const;
    sf::Vector2f getPosition() const;
    sf::Vector2f getSize() const;
    sf::Vector2f getCenter() const;
    Physics::Box getCollisionBox() const;

    void flash(float seconds);
    bool takeDamage(int damage, float flashSeconds);

private:
    struct State
    {
        sf::Vector2f position{};
        sf::Vector2f standingSize{ 64.0f, 96.0f };
        sf::Vector2f guardingSize{ 64.0f, 84.0f };
        float patrolLeftX{ 0.0f };
        float patrolRightX{ 0.0f };
        bool facingRight{ true };
        bool guarding{ false };
        bool alive{ true };
        int health{ 6 };
        float shotTimer{ 0.0f };
        float hitFlashTimer{ 0.0f };
    };

    sf::Vector2f sizeFor(const State& state) const;
    bool lineBlockedByTile(sf::Vector2f from, sf::Vector2f to, Tilemap& tilemap) const;

    State mLive{};
    State mNext{};
    bool mCanSeePlayer{ false };
};

class HealthPickup
{
public:
    enum class CollisionType
    {
        Tile
    };

    struct Collision
    {
        CollisionType type{ CollisionType::Tile };
        float distanceSq{ 0.0f };
        GObj* tile{ nullptr };
    };

    HealthPickup();

    static HealthPickup spawn(sf::Vector2f position);

    void syncCopyFromLive();
    void handleScriptInput(float dt);
    void update(float dt);

    std::vector<Collision> CheckAndStoreCollisions(Tilemap& tilemap);
    void ResolveClosestCollisionsFirst(std::vector<Collision>& collisions);

    void SwapData();
    void render(sf::RenderWindow& window) const;

    bool isAlive() const;
    bool willBeAlive() const;
    bool canBeCollected() const;
    void markCollected();
    Physics::Box getCollisionBox() const;

private:
    struct State
    {
        sf::Vector2f position{};
        sf::Vector2f velocity{};
        float radius{ 14.0f };
        bool alive{ true };
        bool settled{ false };
    };

    State mLive{};
    State mNext{};
};
