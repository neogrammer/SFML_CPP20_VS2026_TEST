#include "PlayState.h"
#include "../Physics.h"

//void handleStaticInput(float dt, GObj* gameObject);

PlayState::PlayState(sf::RenderWindow& wnd)
    : GameState<PlayState>{}
{
    mainView = wnd.getDefaultView();
}

//eStateID PlayState::updateImpl(float dt)
//{
//    // handle static input always first in update for all objects that are controlled by player
//    handleStaticInputImpl(dt, gameObject);
//
//	bool wasGrounded = gameObject->grounded;
//
//	// rebuild grounded fresh this frame during/after collision
//	gameObject->grounded = false;
//
//	if (!wasGrounded)
//	{
//		gameObject->setAccleration({ gameObject->getAcceleration().x, 198.0f });
//	}
//	else
//	{
//		gameObject->setAccleration({ gameObject->getAcceleration().x, 0.f });
//	}
//
//	//if (!gameObject->grounded)
//	//{
//	//	if (gameObject->getAcceleration().y == 0.f)
//	//		gameObject->setVel({ gameObject->getVel().x, gameObject->getVel().y + 100.f });
//	//	gameObject->setAccleration({gameObject->getAcceleration().x, 198.0f});
//	//	
//	//}
//
//
//	gameObject->rightHeld = false;
//	gameObject->leftHeld = false;
//
//	if (gameObject->getVelocity().x < -10.f)
//	{
//		gameObject->leftHeld = true;
//	}
//	
//	if (gameObject->getVelocity().x > 10.f)
//	{
//		gameObject->rightHeld = true;
//	}
//
//    if (mPendingState != eStateID::None && mPendingState != eStateID::Count)
//    {
//        eStateID tmpState = mPendingState;
//        mPendingState = eStateID::None;
//        return tmpState;
//    }
//
//
//
//	ActionIntent intent;
//
//	intent.moveX = 0.f;
//	if (gameObject->leftHeld)
//	{
//		intent.moveX -= 1.f;
//	}
//	if (gameObject->rightHeld) intent.moveX += 1.f;
//
//	intent.jumpPressed = gameObject->jumpPressedThisFrame;
//	intent.jumpHeld = gameObject->jumpHeld;
//	intent.dashPressed = gameObject->dashPressedThisFrame;
//	intent.dashHeld = gameObject->dashHeld;
//	intent.shootPressed = gameObject->shootPressedThisFrame;
//	intent.shootHeld = gameObject->shootHeld;
//
//    // update back buffer for each entity first
//    
//
//    // after all the objects are updated in the back buffer, swap the buffer with the frontbuffer
//	ActionContext ctx;
//	ctx.velocity = gameObject->getVelocity();
//
//	ctx.grounded = gameObject->grounded;
//	ctx.justLeftGround = gameObject->justLeftGround;
//	ctx.justLanded = gameObject->justLanded;
//
//	ctx.dashStarting = gameObject->dashStarting;
//	ctx.dashing = gameObject->dashing;
//	ctx.dashEnding = gameObject->dashEnding;
//
//	ctx.wallKicking = gameObject->wallKicking;
//	ctx.wallLanding = gameObject->wallLanding;
//	ctx.wallSliding = gameObject->wallSliding;
//
//	ctx.teleportingIn = gameObject->teleportingIn;
//	ctx.teleportLanding = gameObject->teleportLanding;
//
//	ctx.shootingLocked = gameObject->weaponIsHoldingShootPose;
//
//
//
//	gameObject->update(dt);
//	Physics::moveFirstOutsideVector(gameObject, tmap->getSolids(), true);
//	gameObject->swapdate();
//
//
//
//
//
//    return eStateID::None;
//    
//}

eStateID PlayState::updateImpl(float dt)
{

    handleStaticInputImpl(dt, gameObject);

    //const bool wasGrounded = gameObject->grounded;
    //gameObject->grounded = false;

    if (!gameObject->grounded)
    {
        gameObject->copy->setAccleration({ gameObject->getAcceleration().x, 198.0f });
    }
    else
    {
        gameObject->copy->setAccleration({ gameObject->getAcceleration().x, 0.f });
        if (gameObject->getVelocity().y > 0.f)
        {
            gameObject->copy->setVel({ gameObject->getVelocity().x, 0.f });
        }
    }


    gameObject->rightHeld = false;
    gameObject->leftHeld = false;

    if (gameObject->getVelocity().x < -10.f)
    {
        gameObject->leftHeld = true;
        

    }

    if (gameObject->getVelocity().x > 10.f)
    {
        gameObject->rightHeld = true;


    }

    if (mPendingState != eStateID::None && mPendingState != eStateID::Count)
    {
        eStateID tmpState = mPendingState;
        mPendingState = eStateID::None;
        return tmpState;
    }


   // Physics::moveFirstOutsideVector(gameObject, tmap->getSolids(), true);

    // move all other entities.. then
    // Sort collisions in order of distance
    sf::Vector2f cp, cn;
    float t = 0, min_t = INFINITY;
    std::vector<std::pair<int, float>> z;

    // Work out collision point, add it to vector along with rect ID
    for (size_t i = 1; i < tmap->getSolids().size(); i++)
    {
        if (Physics::DynamicRectVsRect(gameObject->copy, dt, *tmap->getSolids()[i], cp, cn, t))
        {
            z.push_back({(int) i,(float) t });
        }
    }

    // Do the sort
    std::sort(z.begin(), z.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
        {
            return a.second < b.second;
        });


    // Now resolve the collision in correct order 
    for (auto j : z)
    {
        Physics::ResolveDynamicRectVsRect(gameObject->copy, dt, tmap->getSolids()[j.first]);

    }

    // Embellish the "in contact" rectangles in yellow
    for (int i = 0; i < 4; i++)
    {
        gameObject->copy->contact[i] = nullptr;
    }

    gameObject->update(dt);
    // UPdate the player rectangles position, with its modified velocity
    gameObject->copy->setPos({ gameObject->getPos().x + gameObject->copy->getVelocity().x * dt, gameObject->getPos().y + gameObject->copy->getVelocity().y * dt});
    


    gameObject->swapdate();

    //// ground probe: if 1 px lower would collide, we are grounded
    //const sf::Vector2f probePos = { gameObject->getPos().x, gameObject->getPos().y + 1.f };
    //gameObject->grounded = Physics::overlapsAnyAt(gameObject, tmap->getSolids(), probePos);

    //if (gameObject->grounded && gameObject->getVel().y > 0.f)
    //{
    //    gameObject->setVel({ gameObject->getVel().x, 0.f });
    //}

    return eStateID::None;
}

bool Physics::overlapsAnyAt(
    const GObj* first,
    const std::vector<GObj*>& others,
    const sf::Vector2f& testPos
)
{
    if (first == nullptr)
    {
        return false;
    }

    const Box firstBox = buildBoxFromPositionAndSize(testPos, first->getSizeSafe());

    for (GObj* other : others)
    {
        if (other == nullptr || other == first)
        {
            continue;
        }

        if (boxesOverlap(firstBox, getBox(other)))
        {
            return true;
        }
    }

    return false;
}

//void handleStaticInput(float dt, GObj* gameObject)
//{
//    constexpr float bufferTime = { 0.03f };
//    static float bufferedLeft = 0.f;
//    static float bufferedRight = 0.f;
//    bool d = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
//    bool a = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
//
//    if (d)
//    {
//        bufferedRight += dt;
//    }
//    else
//    {
//        bufferedRight = 0.f;
//    }
//
//    if (a)
//    {
//        bufferedLeft += dt;
//    }
//    else
//    {
//        bufferedLeft = 0.f;
//    }
//
//    auto& c = *dynamic_cast<AnimObj*>(gameObject);
//    if (a || d)
//    {
//        if (a && d)
//        {
//            if (gameObject->getVelocity().x < -0.001f)
//                gameObject->setVel({ -500.f, gameObject->getVel().y });
//            else if (gameObject->getVelocity().x > 0.001f)
//                gameObject->setVel({ 500.f, gameObject->getVel().y });
//
//
//            if (gameObject->getVelocity().x >= -0.001f && gameObject->getVelocity().x <= 0.001f)
//            {
//                if (c.getCurrentAnim() != AnimName::Idle)
//                {
//                    c.setCurrentAnim(AnimName::Idle);
//                }
//            }
//            else
//            {
//                if (c.getCurrentAnim() != AnimName::Run)
//                {
//                    c.setCurrentAnim(AnimName::Run);
//                }
//            }
//
//        }
//        else
//        {
//            if (d && bufferedRight >= bufferTime)
//            {
//                gameObject->setVel({ 500.f, gameObject->getVel().y });
//                gameObject->setFacingRight(true);
//                if (c.getCurrentAnim() != AnimName::Run)
//                {
//                    c.setCurrentAnim(AnimName::Run);
//                }
//
//            }
//            if (a && bufferedLeft >= bufferTime)
//            {
//
//                gameObject->setVel({ -500.f, gameObject->getVel().y });
//                gameObject->setFacingRight(false);
//                if (c.getCurrentAnim() != AnimName::Run)
//                {
//                    c.setCurrentAnim(AnimName::Run);
//                }
//
//            }
//        }
//    }
//    else
//    {
//        if ((!d && !a))
//        {
//            gameObject->setVel({ 0.f, gameObject->getVel().y });
//
//            if (c.getCurrentAnim() != AnimName::Idle)
//            {
//                c.setCurrentAnim(AnimName::Idle);
//            }
//        }
//    }
//
//}