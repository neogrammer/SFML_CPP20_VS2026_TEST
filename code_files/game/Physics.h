#pragma once

#include "GObj.h"
#include <SFML/System/Vector2.hpp>
#include <vector>

class Physics
{
public:
    class Box
    {
    public:
        float left{ 0.f };
        float top{ 0.f };
        float width{ 0.f };
        float height{ 0.f };

    public:
        float right() const;
        float bottom() const;
        float centerX() const;
        float centerY() const;
    };

public:
    // Reads ONLY from the current/front object state.
    static Box getBox(const GObj* obj);

    static bool isOverlapping(const GObj* first, const GObj* second);

    // Moves FIRST out of SECOND using the smallest overlap axis.
    // Reads both from current/front state, writes result to FIRST's copy/back state.
    static bool moveFirstOutsideSecond(
        GObj* first,
        const GObj* second,
        bool zeroVelocityOnResolvedAxis = true
    );

    // Reads all objects from current/front state, sorts blockers by distance
    // from FIRST (nearest to farthest), resolves in that order, and re-checks
    // after movement in case the push causes a new overlap with another object.
    static bool moveFirstOutsideVector(
        GObj* first,
        const std::vector<GObj*>& others,
        bool zeroVelocityOnResolvedAxis = true,
        int maxPasses = 3
    );

private:
    static Box buildBoxFromPositionAndSize(
        const sf::Vector2f& position,
        const sf::Vector2f& size
    );

    static bool boxesOverlap(const Box& first, const Box& second);

    static sf::Vector2f getSeparationVector(
        const Box& first,
        const Box& second,
        bool& resolvedOnX
    );

    static float getDistanceSqBetweenCenters(const Box& first, const Box& second);

    static void writeResolvedPosition(
        GObj* obj,
        const sf::Vector2f& newPosition
    );

    static void writeResolvedPositionAndVelocity(
        GObj* obj,
        const sf::Vector2f& newPosition,
        const sf::Vector2f& newVelocity
    );
};