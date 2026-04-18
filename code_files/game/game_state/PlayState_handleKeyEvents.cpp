#include "PlayState.h"

void PlayState::handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed)
{
	// Player Movement
	if (key == sf::Keyboard::Key::D && isPressed)
	{
		gameObject->setVelCpy({ 500.f, gameObject->getVel().y });
	}
	else if (key == sf::Keyboard::Key::D && !isPressed)
	{
		gameObject->setVelCpy({ 0.f, gameObject->getVel().y });
	}


	if (key == sf::Keyboard::Key::A && isPressed)
	{
		gameObject->setVelCpy({ -500.f, gameObject->getVel().y });
	}
	else if (key == sf::Keyboard::Key::A && !isPressed)
	{
		gameObject->setVelCpy({ 0.f, gameObject->getVel().y });
	}
	// End of player movement
	
}