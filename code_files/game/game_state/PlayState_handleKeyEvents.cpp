#include "PlayState.h"

void PlayState::handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed)
{
	auto& p = *dynamic_cast<AnimObj*>(gameObject);
	// Player Movement
    if (key == sf::Keyboard::Key::D && !isPressed)
	{
		gameObject->setVel({ 0.f, gameObject->getVel().y });

		if (p.getCurrentAnim() != AnimName::Idle)
		{
			p.setCurrentAnim(AnimName::Idle);
		}
	}


	if (key == sf::Keyboard::Key::A && !isPressed)
	{
		gameObject->setVel({ 0.f, gameObject->getVel().y });

		if (p.getCurrentAnim() != AnimName::Idle)
		{
			p.setCurrentAnim(AnimName::Idle);
		}
	}
	// End of player movement
	
}