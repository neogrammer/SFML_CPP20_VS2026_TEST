#include "PlayState.h"

void PlayState::renderImpl(sf::RenderWindow& window)
{
	tmap->renderMap(window);

	window.draw(*gameObject->sprite());
}