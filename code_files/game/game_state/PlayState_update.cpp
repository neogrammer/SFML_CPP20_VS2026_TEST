#include "PlayState.h"
#include "../Physics.h"

//void handleStaticInput(float dt, GObj* gameObject);

PlayState::PlayState(sf::RenderWindow& wnd)
    : GameState<PlayState>{}
{
    mainView = wnd.getDefaultView();
}

eStateID PlayState::updateImpl(float dt)
{
    // handle static input always first in update for all objects that are controlled by player
    handleStaticInputImpl(dt, gameObject);

	if (!gameObject->grounded)
	{
		gameObject->setVel({ gameObject->getVelocity().x, 100.f});
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



	ActionIntent intent;

	intent.moveX = 0.f;
	if (gameObject->leftHeld)
	{
		intent.moveX -= 1.f;
	}
	if (gameObject->rightHeld) intent.moveX += 1.f;

	intent.jumpPressed = gameObject->jumpPressedThisFrame;
	intent.jumpHeld = gameObject->jumpHeld;
	intent.dashPressed = gameObject->dashPressedThisFrame;
	intent.dashHeld = gameObject->dashHeld;
	intent.shootPressed = gameObject->shootPressedThisFrame;
	intent.shootHeld = gameObject->shootHeld;

    // update back buffer for each entity first
    

    // after all the objects are updated in the back buffer, swap the buffer with the frontbuffer
	ActionContext ctx;
	ctx.velocity = gameObject->getVelocity();

	ctx.grounded = gameObject->grounded;
	ctx.justLeftGround = gameObject->justLeftGround;
	ctx.justLanded = gameObject->justLanded;

	ctx.dashStarting = gameObject->dashStarting;
	ctx.dashing = gameObject->dashing;
	ctx.dashEnding = gameObject->dashEnding;

	ctx.wallKicking = gameObject->wallKicking;
	ctx.wallLanding = gameObject->wallLanding;
	ctx.wallSliding = gameObject->wallSliding;

	ctx.teleportingIn = gameObject->teleportingIn;
	ctx.teleportLanding = gameObject->teleportLanding;

	ctx.shootingLocked = gameObject->weaponIsHoldingShootPose;

	//mActMgr.update(*dynamic_cast<AnimObj*>(gameObject), intent, ctx);
	gameObject->update(dt);






	Physics::moveFirstOutsideVector(gameObject, tmap->getSolids());
    gameObject->swapdate();







    return eStateID::None;
    
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