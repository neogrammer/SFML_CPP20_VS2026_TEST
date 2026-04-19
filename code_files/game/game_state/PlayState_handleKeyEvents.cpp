#include "PlayState.h"

void PlayState::handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed)
{
	auto& p = *dynamic_cast<AnimObj*>(gameObject);
	//// Player Movement :  Stop on flag on top of other static movement
    if (key == sf::Keyboard::Key::D && !isPressed && p.getVelocity().x > 0.001f)
	{
		gameObject->setVel({ 0.f, gameObject->getVel().y });

		if (p.getCurrentAnim() != AnimName::Idle)
		{
			p.setCurrentAnim(AnimName::Idle);
		}
	}


	if (key == sf::Keyboard::Key::A && !isPressed && p.getVelocity().x < -0.001f)
	{
		gameObject->setVel({ 0.f, gameObject->getVel().y });

		if (p.getCurrentAnim() != AnimName::Idle)
		{
			p.setCurrentAnim(AnimName::Idle);
		}
	}
	// End of player movement
	
}