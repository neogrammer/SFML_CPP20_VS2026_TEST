#include "CombatEntities.h"

#include "Player.h"
#include "../map/Tilemap.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float PlayerShotSpeed = 1350.0f;
    constexpr float EnemyShotSpeed = 920.0f;
    constexpr float EnemyShotDelay = 2.5f;
    constexpr float EnemySightDistance = 600.0f;
    constexpr float EnemyWalkSpeed = 120.0f;
    constexpr int EnemyShotDamage = 2;
    constexpr float HitFlashSeconds = 0.12f;
    constexpr float HealthPickupGravity = 1450.0f;

    Physics::Box makeBox(const sf::Vector2f& position, const sf::Vector2f& size)
    {
        Physics::Box box;
        box.left = position.x;
        box.top = position.y;
        box.width = size.x;
        box.height = size.y;
        return box;
    }

    Physics::Box makeCircleBox(const sf::Vector2f& position, float radius)
    {
        return makeBox(
            { position.x - radius, position.y - radius },
            { radius * 2.0f, radius * 2.0f }
        );
    }

    bool boxesOverlap(const Physics::Box& first, const Physics::Box& second)
    {
        if (first.width <= 0.0f || first.height <= 0.0f || second.width <= 0.0f || second.height <= 0.0f)
        {
            return false;
        }

        return first.left < second.right() &&
            first.right() > second.left &&
            first.top < second.bottom() &&
            first.bottom() > second.top;
    }

    sf::Vector2f boxCenter(const sf::Vector2f& position, const sf::Vector2f& size)
    {
        return { position.x + (size.x * 0.5f), position.y + (size.y * 0.5f) };
    }

    float distanceSq(sf::Vector2f a, sf::Vector2f b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        return (dx * dx) + (dy * dy);
    }

    sf::Vector2f normalized(sf::Vector2f value)
    {
        const float lengthSq = (value.x * value.x) + (value.y * value.y);
        if (lengthSq <= 0.0001f)
        {
            return { 1.0f, 0.0f };
        }

        const float length = std::sqrt(lengthSq);
        return { value.x / length, value.y / length };
    }

    bool pointInsideBox(sf::Vector2f point, const Physics::Box& box)
    {
        return point.x >= box.left &&
            point.x <= box.right() &&
            point.y >= box.top &&
            point.y <= box.bottom();
    }

    Physics::Box screenBoxFromView(const sf::View& view)
    {
        const sf::Vector2f viewCenter = view.getCenter();
        const sf::Vector2f viewSize = view.getSize();
        return makeBox(
            { viewCenter.x - (viewSize.x * 0.5f) - 96.0f, viewCenter.y - (viewSize.y * 0.5f) - 96.0f },
            { viewSize.x + 192.0f, viewSize.y + 192.0f }
        );
    }
}

EnergyShot::EnergyShot()
{
    syncCopyFromLive();
}

EnergyShot EnergyShot::makePlayerShot(Player& player)
{
    EnergyShot shot;
    const bool right = player.isFacingRight();
    shot.mLive.radius = 7.0f;
    shot.mLive.position = player.getShotSpawnPosition(shot.mLive.radius);
    shot.mLive.velocity = { right ? PlayerShotSpeed : -PlayerShotSpeed, 0.0f };
    shot.mLive.fromPlayer = true;
    shot.mLive.canDamagePlayer = false;
    shot.mLive.color = sf::Color(90, 235, 255);
    shot.syncCopyFromLive();
    return shot;
}

EnergyShot EnergyShot::makeEnemyShot(sf::Vector2f position, sf::Vector2f target)
{
    EnergyShot shot;
    shot.mLive.radius = 7.0f;
    shot.mLive.fromPlayer = false;
    shot.mLive.canDamagePlayer = true;
    shot.mLive.color = sf::Color(255, 70, 70);
    shot.mLive.position = position;
    shot.mLive.velocity = normalized(target - position) * EnemyShotSpeed;
    shot.syncCopyFromLive();
    return shot;
}

void EnergyShot::syncCopyFromLive()
{
    mNext = mLive;
}

void EnergyShot::handleScriptInput(float dt)
{
    (void)dt;
}

void EnergyShot::update(float dt, const sf::View& view)
{
    (void)view;

    if (!mNext.alive)
    {
        return;
    }

    mNext.position += mNext.velocity * dt;
}

std::vector<EnergyShot::Collision> EnergyShot::CheckAndStoreCollisions(
    Tilemap& tilemap,
    const sf::View& view,
    Player& player,
    std::vector<GuardEnemy>& enemies
)
{
    std::vector<Collision> collisions;

    if (!mNext.alive)
    {
        return collisions;
    }

    const Physics::Box shotBox = makeCircleBox(mNext.position, mNext.radius);
    if (!boxesOverlap(shotBox, screenBoxFromView(view)))
    {
        Collision collision;
        collision.type = CollisionType::ScreenExit;
        collision.distanceSq = -1.0f;
        collisions.emplace_back(collision);
        return collisions;
    }

    const sf::Vector2f shotCenter = mNext.position;
    for (GObj* solid : tilemap.getSolids())
    {
        if (solid == nullptr)
        {
            continue;
        }

        const Physics::Box tileBox = Physics::getBox(solid);
        if (!boxesOverlap(shotBox, tileBox))
        {
            continue;
        }

        Collision collision;
        collision.type = CollisionType::Tile;
        collision.tile = solid;
        collision.distanceSq = distanceSq(shotCenter, { tileBox.centerX(), tileBox.centerY() });
        collisions.emplace_back(collision);
    }

    if (mNext.fromPlayer)
    {
        for (GuardEnemy& enemy : enemies)
        {
            if (!enemy.willBeAlive())
            {
                continue;
            }

            const Physics::Box enemyBox = enemy.getCollisionBox();
            if (!boxesOverlap(shotBox, enemyBox))
            {
                continue;
            }

            Collision collision;
            collision.type = CollisionType::Enemy;
            collision.enemy = &enemy;
            collision.distanceSq = distanceSq(shotCenter, { enemyBox.centerX(), enemyBox.centerY() });
            collisions.emplace_back(collision);
        }
    }
    else if (mNext.canDamagePlayer)
    {
        const Physics::Box playerBox = makeBox(player.getPosSafe(), player.getSizeSafe());
        if (boxesOverlap(shotBox, playerBox))
        {
            Collision collision;
            collision.type = CollisionType::Player;
            collision.player = &player;
            collision.distanceSq = distanceSq(shotCenter, { playerBox.centerX(), playerBox.centerY() });
            collisions.emplace_back(collision);
        }
    }

    return collisions;
}

void EnergyShot::ResolveClosestCollisionsFirst(
    std::vector<Collision>& collisions,
    Player& player,
    std::vector<GuardEnemy>& enemies,
    std::vector<HealthPickup>& healthPickups
)
{
    (void)player;
    (void)enemies;

    if (!mNext.alive || collisions.empty())
    {
        return;
    }

    std::sort(
        collisions.begin(),
        collisions.end(),
        [](const Collision& left, const Collision& right)
        {
            return left.distanceSq < right.distanceSq;
        }
    );

    for (const Collision& collision : collisions)
    {
        if (!mNext.alive)
        {
            return;
        }

        if (collision.type == CollisionType::ScreenExit || collision.type == CollisionType::Tile)
        {
            mNext.alive = false;
            return;
        }

        if (collision.type == CollisionType::Player && collision.player != nullptr)
        {
            collision.player->takeDamage(EnemyShotDamage, mNext.position);
            mNext.alive = false;
            return;
        }

        if (collision.type == CollisionType::Enemy && collision.enemy != nullptr)
        {
            GuardEnemy& enemy = *collision.enemy;
            const Physics::Box enemyBox = enemy.getCollisionBox();

            if (enemy.isGuarding())
            {
                enemy.flash(HitFlashSeconds);
                mNext.fromPlayer = false;
                mNext.canDamagePlayer = false;

                const float speed = std::max(
                    PlayerShotSpeed,
                    std::sqrt((mNext.velocity.x * mNext.velocity.x) + (mNext.velocity.y * mNext.velocity.y))
                );
                constexpr float InvSqrt2 = 0.70710678f;
                const float xDirection = mNext.velocity.x > 0.0f ? -1.0f : 1.0f;
                mNext.velocity = { xDirection * speed * InvSqrt2, -speed * InvSqrt2 };
                mNext.color = sf::Color(255, 230, 70);

                if (mNext.velocity.x > 0.0f)
                {
                    mNext.position.x = enemyBox.right() + mNext.radius + 1.0f;
                }
                else
                {
                    mNext.position.x = enemyBox.left - mNext.radius - 1.0f;
                }

                return;
            }

            const bool enemyDied = enemy.takeDamage(1, HitFlashSeconds);
            mNext.alive = false;

            if (enemyDied)
            {
                healthPickups.emplace_back(HealthPickup::spawn(enemy.getCenter()));
            }

            return;
        }
    }
}

void EnergyShot::SwapData()
{
    mLive = mNext;
}

void EnergyShot::render(sf::RenderWindow& window) const
{
    if (!mLive.alive)
    {
        return;
    }

    sf::CircleShape circle{ mLive.radius, 20 };
    circle.setPosition({ mLive.position.x - mLive.radius, mLive.position.y - mLive.radius });
    circle.setFillColor(mLive.color);
    circle.setOutlineColor(sf::Color::White);
    circle.setOutlineThickness(1.0f);
    window.draw(circle);
}

bool EnergyShot::isAlive() const
{
    return mLive.alive;
}

bool EnergyShot::willBeAlive() const
{
    return mLive.alive && mNext.alive;
}

bool EnergyShot::isFromPlayer() const
{
    return mLive.fromPlayer;
}

Physics::Box EnergyShot::getCollisionBox() const
{
    return makeCircleBox(mLive.position, mLive.radius);
}

GuardEnemy::GuardEnemy()
{
    syncCopyFromLive();
}

GuardEnemy GuardEnemy::makePatroller(
    sf::Vector2f position,
    float patrolLeftX,
    float patrolRightX,
    bool facingRight,
    float firstShotDelay
)
{
    GuardEnemy enemy;
    enemy.mLive.position = position;
    enemy.mLive.patrolLeftX = patrolLeftX;
    enemy.mLive.patrolRightX = patrolRightX;
    enemy.mLive.facingRight = facingRight;
    enemy.mLive.shotTimer = firstShotDelay;
    enemy.syncCopyFromLive();
    return enemy;
}

void GuardEnemy::syncCopyFromLive()
{
    mNext = mLive;
}

void GuardEnemy::handleScriptInput(Player& player, Tilemap& tilemap)
{
    if (!mLive.alive)
    {
        mCanSeePlayer = false;
        return;
    }

    const sf::Vector2f target = boxCenter(player.getPosSafe(), player.getSizeSafe());
    const sf::Vector2f eyes = getCenter();
    const sf::Vector2f toPlayer = target - eyes;

    const bool playerIsInFront = mLive.facingRight ? toPlayer.x > 0.0f : toPlayer.x < 0.0f;
    const bool playerIsClose = std::fabs(toPlayer.x) <= EnemySightDistance;

    mCanSeePlayer =
        playerIsInFront &&
        playerIsClose &&
        !lineBlockedByTile(eyes, target, tilemap);
}

void GuardEnemy::update(float dt, Player& player, std::vector<EnergyShot>& enemyShots)
{
    if (!mNext.alive)
    {
        return;
    }

    mNext.hitFlashTimer = std::max(0.0f, mNext.hitFlashTimer - dt);

    const sf::Vector2f oldSize = sizeFor(mLive);
    const float feetY = mLive.position.y + oldSize.y;
    const sf::Vector2f target = boxCenter(player.getPosSafe(), player.getSizeSafe());

    if (mNext.guarding != mCanSeePlayer)
    {
        mNext.guarding = mCanSeePlayer;
        mNext.position.y = feetY - sizeFor(mNext).y;

        if (mNext.guarding)
        {
            mNext.shotTimer = std::min(mNext.shotTimer, 0.35f);
        }
    }

    if (mNext.guarding)
    {
        mNext.shotTimer -= dt;
        if (mNext.shotTimer <= 0.0f)
        {
            sf::Vector2f shotPosition = boxCenter(mNext.position, sizeFor(mNext));
            shotPosition.x += mNext.facingRight ? sizeFor(mNext).x * 0.55f : -sizeFor(mNext).x * 0.55f;
            shotPosition.y -= 8.0f;
            enemyShots.emplace_back(EnergyShot::makeEnemyShot(shotPosition, target));

            mNext.shotTimer = EnemyShotDelay;
        }

        return;
    }

    const float direction = mNext.facingRight ? 1.0f : -1.0f;
    mNext.position.x += direction * EnemyWalkSpeed * dt;

    if (mNext.position.x >= mNext.patrolRightX)
    {
        mNext.position.x = mNext.patrolRightX;
        mNext.facingRight = false;
    }
    else if (mNext.position.x <= mNext.patrolLeftX)
    {
        mNext.position.x = mNext.patrolLeftX;
        mNext.facingRight = true;
    }
}

std::vector<GuardEnemy::Collision> GuardEnemy::CheckAndStoreCollisions(Tilemap& tilemap, std::vector<EnergyShot>& playerShots)
{
    (void)tilemap;
    (void)playerShots;
    return {};
}

void GuardEnemy::ResolveClosestCollisionsFirst(std::vector<Collision>& collisions)
{
    (void)collisions;
}

void GuardEnemy::SwapData()
{
    mLive = mNext;
}

void GuardEnemy::render(sf::RenderWindow& window) const
{
    if (!mLive.alive)
    {
        return;
    }

    sf::RectangleShape body;
    body.setPosition(mLive.position);
    body.setSize(sizeFor(mLive));

    const bool flashing = mLive.hitFlashTimer > 0.0f;
    body.setFillColor(flashing ? sf::Color::White : (mLive.guarding ? sf::Color(180, 20, 20) : sf::Color(235, 35, 35)));
    body.setOutlineColor(flashing ? sf::Color::White : (mLive.guarding ? sf::Color(255, 230, 60) : sf::Color(70, 0, 0)));
    body.setOutlineThickness(3.0f);
    window.draw(body);

    sf::RectangleShape eye;
    eye.setSize({ 9.0f, 9.0f });
    eye.setFillColor(sf::Color::White);
    eye.setPosition({
        mLive.position.x + (mLive.facingRight ? sizeFor(mLive).x - 18.0f : 9.0f),
        mLive.position.y + 18.0f
    });
    window.draw(eye);
}

bool GuardEnemy::isAlive() const
{
    return mLive.alive;
}

bool GuardEnemy::willBeAlive() const
{
    return mLive.alive && mNext.alive;
}

bool GuardEnemy::isGuarding() const
{
    return mLive.guarding;
}

bool GuardEnemy::isFacingRight() const
{
    return mLive.facingRight;
}

sf::Vector2f GuardEnemy::getPosition() const
{
    return mLive.position;
}

sf::Vector2f GuardEnemy::getSize() const
{
    return sizeFor(mLive);
}

sf::Vector2f GuardEnemy::getCenter() const
{
    return boxCenter(mLive.position, sizeFor(mLive));
}

Physics::Box GuardEnemy::getCollisionBox() const
{
    return makeBox(mLive.position, sizeFor(mLive));
}

void GuardEnemy::flash(float seconds)
{
    mNext.hitFlashTimer = std::max(mNext.hitFlashTimer, seconds);
}

bool GuardEnemy::takeDamage(int damage, float flashSeconds)
{
    if (!mNext.alive)
    {
        return false;
    }

    flash(flashSeconds);
    mNext.health -= damage;

    if (mNext.health > 0)
    {
        return false;
    }

    mNext.alive = false;
    return true;
}

sf::Vector2f GuardEnemy::sizeFor(const State& state) const
{
    return state.guarding ? state.guardingSize : state.standingSize;
}

bool GuardEnemy::lineBlockedByTile(sf::Vector2f from, sf::Vector2f to, Tilemap& tilemap) const
{
    const sf::Vector2f delta = to - from;
    const float distance = std::sqrt((delta.x * delta.x) + (delta.y * delta.y));
    const int steps = std::max(1, static_cast<int>(distance / 32.0f));

    for (int i = 1; i < steps; ++i)
    {
        const float pct = static_cast<float>(i) / static_cast<float>(steps);
        const sf::Vector2f point = from + (delta * pct);

        for (GObj* solid : tilemap.getSolids())
        {
            if (solid != nullptr && pointInsideBox(point, Physics::getBox(solid)))
            {
                return true;
            }
        }
    }

    return false;
}

HealthPickup::HealthPickup()
{
    syncCopyFromLive();
}

HealthPickup HealthPickup::spawn(sf::Vector2f position)
{
    HealthPickup pickup;
    pickup.mLive.position = position;
    pickup.mLive.velocity = { 0.0f, -160.0f };
    pickup.syncCopyFromLive();
    return pickup;
}

void HealthPickup::syncCopyFromLive()
{
    mNext = mLive;
}

void HealthPickup::handleScriptInput(float dt)
{
    (void)dt;
}

void HealthPickup::update(float dt)
{
    if (!mNext.alive || mNext.settled)
    {
        return;
    }

    mNext.velocity.y += HealthPickupGravity * dt;
    mNext.position += mNext.velocity * dt;
}

std::vector<HealthPickup::Collision> HealthPickup::CheckAndStoreCollisions(Tilemap& tilemap)
{
    std::vector<Collision> collisions;

    if (!mNext.alive || mNext.settled)
    {
        return collisions;
    }

    const Physics::Box pickupBox = makeCircleBox(mNext.position, mNext.radius);

    for (GObj* solid : tilemap.getSolids())
    {
        if (solid == nullptr)
        {
            continue;
        }

        const Physics::Box tileBox = Physics::getBox(solid);
        const bool fallingOntoTile =
            mNext.velocity.y >= 0.0f &&
            pickupBox.bottom() >= tileBox.top &&
            pickupBox.top < tileBox.top &&
            pickupBox.right() > tileBox.left &&
            pickupBox.left < tileBox.right();

        if (!fallingOntoTile)
        {
            continue;
        }

        Collision collision;
        collision.type = CollisionType::Tile;
        collision.tile = solid;
        collision.distanceSq = distanceSq(mNext.position, { tileBox.centerX(), tileBox.centerY() });
        collisions.emplace_back(collision);
    }

    return collisions;
}

void HealthPickup::ResolveClosestCollisionsFirst(std::vector<Collision>& collisions)
{
    if (!mNext.alive || collisions.empty())
    {
        return;
    }

    std::sort(
        collisions.begin(),
        collisions.end(),
        [](const Collision& left, const Collision& right)
        {
            return left.distanceSq < right.distanceSq;
        }
    );

    const Collision& collision = collisions.front();
    if (collision.tile == nullptr)
    {
        return;
    }

    const Physics::Box tileBox = Physics::getBox(collision.tile);
    mNext.position.y = tileBox.top - mNext.radius - 1.0f;
    mNext.velocity = { 0.0f, 0.0f };
    mNext.settled = true;
}

void HealthPickup::SwapData()
{
    mLive = mNext;
}

void HealthPickup::render(sf::RenderWindow& window) const
{
    if (!mLive.alive)
    {
        return;
    }

    sf::CircleShape orb{ mLive.radius, 28 };
    orb.setPosition({ mLive.position.x - mLive.radius, mLive.position.y - mLive.radius });
    orb.setFillColor(sf::Color(255, 220, 40));
    orb.setOutlineColor(sf::Color(255, 255, 210));
    orb.setOutlineThickness(3.0f);
    window.draw(orb);

    sf::CircleShape shine{ mLive.radius * 0.35f, 16 };
    shine.setPosition({
        mLive.position.x - (mLive.radius * 0.55f),
        mLive.position.y - (mLive.radius * 0.65f)
    });
    shine.setFillColor(sf::Color(255, 255, 255, 170));
    window.draw(shine);
}

bool HealthPickup::isAlive() const
{
    return mLive.alive;
}

bool HealthPickup::willBeAlive() const
{
    return mLive.alive && mNext.alive;
}

bool HealthPickup::canBeCollected() const
{
    return willBeAlive();
}

void HealthPickup::markCollected()
{
    mNext.alive = false;
}

Physics::Box HealthPickup::getCollisionBox() const
{
    return makeCircleBox(mLive.position, mLive.radius);
}
