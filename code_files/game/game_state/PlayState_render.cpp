#include "PlayState.h"

void PlayState::renderImpl(sf::RenderWindow& window)
{
	window.draw(*gameObject->sprite());
}