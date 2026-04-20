#include "Physics.h"
#include <algorithm>
#include <cmath>

float Physics::Box::right() const
{
    return left + width;
}

float Physics::Box::bottom() const
{
    return top + height;
}

float Physics::Box::centerX() const
{
    return left + (width * 0.5f);
}

float Physics::Box::centerY() const
{
    return top + (height * 0.5f);
}

Physics::Box Physics::getBox(const GObj* obj)
{
    if (obj == nullptr)
    {
        return Box{};
    }

    return buildBoxFromPositionAndSize(obj->getPosSafe(), obj->getSizeSafe());
}

bool Physics::isOverlapping(const GObj* first, const GObj* second)
{
    if (first == nullptr || second == nullptr)
    {
        return false;
    }

    return boxesOverlap(getBox(first), getBox(second));
}



bool Physics::moveFirstOutsideSecond(
    GObj* first,
    const GObj* second,
    bool zeroVelocityOnResolvedAxis
)
{
    if (first == nullptr || second == nullptr)
    {
        return false;
    }

    if (first == second)
    {
        return false;
    }

    const GObj* moverState = (first->copy != nullptr) ? first->copy : first;

    const sf::Vector2f workingPos = moverState->getPosSafe();
    const sf::Vector2f workingVel = moverState->getVelSafe();
    const sf::Vector2f workingSize = moverState->getSizeSafe();

    const Box firstBox = buildBoxFromPositionAndSize(workingPos, workingSize);
    const Box secondBox = getBox(second); // blocker stays on current/live state

    if (!boxesOverlap(firstBox, secondBox))
    {
        return false;
    }

    bool resolvedOnX = false;
    const sf::Vector2f separation = getSeparationVector(firstBox, secondBox, resolvedOnX);

    sf::Vector2f newPosition = workingPos + separation;

    if (!zeroVelocityOnResolvedAxis)
    {
        writeResolvedPosition(first, newPosition);
        return true;
    }

    sf::Vector2f newVelocity = workingVel;

    if (resolvedOnX)
    {
        newVelocity.x = 0.f;
    }
    else
    {
        newVelocity.y = 0.f;
    }

    writeResolvedPositionAndVelocity(first, newPosition, newVelocity);
    return true;
}

bool Physics::moveFirstOutsideVector(
    GObj* first,
    const std::vector<GObj*>& others,
    bool zeroVelocityOnResolvedAxis,
    int maxPasses
)
{
    if (first == nullptr || others.empty())
    {
        return false;
    }

    std::vector<GObj*> sorted;
    sorted.reserve(others.size());

    for (GObj* other : others)
    {
        if (other == nullptr)
        {
            continue;
        }

        if (other == first)
        {
            continue;
        }

        sorted.emplace_back(other);
    }

    if (sorted.empty())
    {
        return false;
    }

    const GObj* moverState = (first->copy != nullptr) ? first->copy : first;

    const sf::Vector2f firstStartPos = moverState->getPosSafe();
    const sf::Vector2f firstStartVel = moverState->getVelSafe();
    const sf::Vector2f firstSize = moverState->getSizeSafe();
    const Box firstStartBox = buildBoxFromPositionAndSize(firstStartPos, firstSize);

    std::sort(
        sorted.begin(),
        sorted.end(),
        [&](const GObj* a, const GObj* b)
        {
            const float distA = getDistanceSqBetweenCenters(firstStartBox, getBox(a));
            const float distB = getDistanceSqBetweenCenters(firstStartBox, getBox(b));
            return distA < distB;
        }
    );

    if (maxPasses < 1)
    {
        maxPasses = 1;
    }

    sf::Vector2f workingPos = firstStartPos;
    sf::Vector2f workingVel = firstStartVel;

    bool movedAnything = false;

    for (int pass = 0; pass < maxPasses; ++pass)
    {
        bool movedThisPass = false;

        for (GObj* other : sorted)
        {
            const Box moverBox = buildBoxFromPositionAndSize(workingPos, firstSize);
            const Box blockerBox = getBox(other); // blocker stays on current/live state

            if (!boxesOverlap(moverBox, blockerBox))
            {
                continue;
            }

            bool resolvedOnX = false;
            const sf::Vector2f separation = getSeparationVector(moverBox, blockerBox, resolvedOnX);

            workingPos += separation;

            if (zeroVelocityOnResolvedAxis)
            {
                if (resolvedOnX)
                {
                    workingVel.x = 0.f;
                }
                else
                {
                    workingVel.y = 0.f;
                }
            }

            movedAnything = true;
            movedThisPass = true;
        }

        if (!movedThisPass)
        {
            break;
        }
    }

    if (!movedAnything)
    {
        return false;
    }

    if (zeroVelocityOnResolvedAxis)
    {
        writeResolvedPositionAndVelocity(first, workingPos, workingVel);
    }
    else
    {
        writeResolvedPosition(first, workingPos);
    }

    return true;
}

Physics::Box Physics::buildBoxFromPositionAndSize(
    const sf::Vector2f& position,
    const sf::Vector2f& size
)
{
    Box out;
    out.left = position.x;
    out.top = position.y;
    out.width = size.x;
    out.height = size.y;
    return out;
}

bool Physics::boxesOverlap(const Box& first, const Box& second)
{
    if (first.width <= 0.f || first.height <= 0.f || second.width <= 0.f || second.height <= 0.f)
    {
        return false;
    }

    if (first.right() <= second.left)
    {
        return false;
    }

    if (first.left >= second.right())
    {
        return false;
    }

    if (first.bottom() <= second.top)
    {
        return false;
    }

    if (first.top >= second.bottom())
    {
        return false;
    }

    return true;
}

sf::Vector2f Physics::getSeparationVector(
    const Box& first,
    const Box& second,
    bool& resolvedOnX
)
{
    const float firstHalfWidth = first.width * 0.5f;
    const float firstHalfHeight = first.height * 0.5f;
    const float secondHalfWidth = second.width * 0.5f;
    const float secondHalfHeight = second.height * 0.5f;

    const float dx = first.centerX() - second.centerX();
    const float dy = first.centerY() - second.centerY();

    const float px = (firstHalfWidth + secondHalfWidth) - std::fabs(dx);
    const float py = (firstHalfHeight + secondHalfHeight) - std::fabs(dy);

    if (px <= 0.f || py <= 0.f)
    {
        resolvedOnX = true;
        return { 0.f, 0.f };
    }

    if (px < py)
    {
        resolvedOnX = true;

        if (dx < 0.f)
        {
            return { -px, 0.f };
        }

        return { px, 0.f };
    }

    resolvedOnX = false;

    if (dy < 0.f)
    {
        return { 0.f, -py };
    }

    return { 0.f, py };
}

float Physics::getDistanceSqBetweenCenters(const Box& first, const Box& second)
{
    const float dx = first.centerX() - second.centerX();
    const float dy = first.centerY() - second.centerY();
    return (dx * dx) + (dy * dy);
}

void Physics::writeResolvedPosition(
    GObj* obj,
    const sf::Vector2f& newPosition
)
{
    if (obj == nullptr)
    {
        return;
    }

    if (obj->copy != nullptr)
    {
        obj->setPosCpy(newPosition);
        return;
    }

    obj->setPos(newPosition);
}

void Physics::writeResolvedPositionAndVelocity(
    GObj* obj,
    const sf::Vector2f& newPosition,
    const sf::Vector2f& newVelocity
)
{
    if (obj == nullptr)
    {
        return;
    }

    if (obj->copy != nullptr)
    {
        obj->setPosCpy(newPosition);
        obj->setVelCpy(newVelocity);
        return;
    }

    obj->setPos(newPosition);
    obj->setVel(newVelocity);
}