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
                    //if (separation.y < 0.f)
                    //{
                    //    first->grounded = true;
                    //}
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

bool Physics::PointVsRect(const GObj& p, const GObj* r)
{
    return (p.position.x >= r->position.x && p.position.y >= r->position.y && p.position.x < r->position.x + r->size.x && p.position.y < r->position.y + r->size.y);
}


bool Physics::RectVsRect(const GObj* r1, const GObj* r2)
{
    return (r1->position.x < r2->position.x + r2->size.x && r1->position.x + r1->size.x > r2->position.x && r1->position.y < r2->position.y + r2->size.y && r1->position.y + r1->size.y > r2->position.y);
}

bool Physics::RayVsRect(const sf::Vector2f& ray_origin, const sf::Vector2f& ray_dir, const GObj* target, sf::Vector2f& contact_point, sf::Vector2f& contact_normal, float& t_hit_near)
{
    contact_normal = { 0,0 };
    contact_point = { 0,0 };

    // Cache division
    sf::Vector2f invdir = { 1.0f / ray_dir.x, 1.0f / ray_dir.y };

    // Calculate intersections with rectangle bounding axes
    sf::Vector2f t_near = { (target->position.x - ray_origin.x) * invdir.x, (target->position.y - ray_origin.y) * invdir.y};
    sf::Vector2f t_far = { (target->position.x + target->size.x - ray_origin.x) * invdir.x, (target->position.y + target->size.y - ray_origin.y) * invdir.y };

    if (std::isnan(t_far.y) || std::isnan(t_far.x)) return false;
    if (std::isnan(t_near.y) || std::isnan(t_near.x)) return false;

    // Sort distances
    if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
    if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

    // Early rejection		
    if (t_near.x > t_far.y || t_near.y > t_far.x) return false;

    // Closest 'time' will be the first contact
    t_hit_near = std::max(t_near.x, t_near.y);

    // Furthest 'time' is contact on opposite side of target
    float t_hit_far = std::min(t_far.x, t_far.y);

    // Reject if ray direction is pointing away from object
    if (t_hit_far < 0)
        return false;

    // Contact point of collision from parametric line equation
    contact_point = { ray_origin.x + t_hit_near * ray_dir.x,  ray_origin.y + t_hit_near * ray_dir.y };

    if (t_near.x > t_near.y)
        if (invdir.x < 0)
            contact_normal = { 1, 0 };
        else
            contact_normal = { -1, 0 };
    else if (t_near.x < t_near.y)
        if (invdir.y < 0)
            contact_normal = { 0, 1 };
        else
            contact_normal = { 0, -1 };

    // Note if t_near == t_far, collision is principly in a diagonal
    // so pointless to resolve. By returning a CN={0,0} even though its
    // considered a hit, the resolver wont change anything.
    return true;
}



bool Physics::DynamicRectVsRect(const GObj* r_dynamic, const float fTimeStep, GObj& r_static, sf::Vector2f& contact_point, sf::Vector2f& contact_normal, float& contact_time)
{
    // Check if dynamic rectangle is actually moving - we assume rectangles are NOT in collision to start
    if (r_dynamic->velocity.x == 0 && r_dynamic->velocity.y == 0)
        return false;

    // Expand target rectangle by source dimensions
    GObj expanded_target{ r_static.getTexID(), r_static.getRect(), r_static.isUniDirectional(), {r_static.position.x - r_dynamic->size.x / 2.f,r_static.position.y - r_dynamic->size.y / 2.f}, {r_static.size + r_dynamic->size}, {  r_static.getOffset().x + r_dynamic->size.x / 2.f, r_static.getOffset().y + r_dynamic->size.y / 2.f} };
    //expanded_target.pos = r_static.pos - r_dynamic->size / 2;
    //expanded_target.size = r_static.size + r_dynamic->size;

    if (RayVsRect({ r_dynamic->position.x + (r_dynamic->size.x / 2.f), r_dynamic->position.y + (r_dynamic->size.y / 2.f) }, { r_dynamic->velocity.x * fTimeStep, r_dynamic->velocity.y * fTimeStep }, &expanded_target, contact_point, contact_normal, contact_time))
        return (contact_time >= 0.0f && contact_time < 1.0f);
    else
        return false;
}

bool Physics::ResolveDynamicRectVsRect(GObj* r_dynamic, const float fTimeStep, GObj* r_static)
{
    sf::Vector2f contact_point, contact_normal;
    float contact_time = 0.0f;
    
    if (DynamicRectVsRect(r_dynamic, fTimeStep, *r_static, contact_point, contact_normal, contact_time))
    {
        if (contact_normal.y > 0) r_dynamic->contact[0] = r_static; else nullptr;
        if (contact_normal.x < 0) r_dynamic->contact[1] = r_static; else nullptr;
        if (contact_normal.y < 0) r_dynamic->contact[2] = r_static; else nullptr;
        if (contact_normal.x > 0) r_dynamic->contact[3] = r_static; else nullptr;

        r_dynamic->velocity.x = r_dynamic->velocity.x + contact_normal.x * std::abs(r_dynamic->velocity.x) * (1 - contact_time);
        r_dynamic->velocity.y = r_dynamic->velocity.y + contact_normal.y * std::abs(r_dynamic->velocity.y) * (1 - contact_time);

        if (contact_normal.y < 0)
            r_dynamic->grounded = true;

        return true;
    }

    return false;
}