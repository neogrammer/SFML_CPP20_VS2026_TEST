#include "PlayState.h"

void PlayState::renderImpl(sf::RenderWindow& window)
{




	static float prevCenterX{ 0.f };
	auto centerX = gameObject->getPos().x + (gameObject->getSize().x / 2.f);
	if (window.mapCoordsToPixel({ centerX,0.f }).x > 800 && gameObject->isFacingRight() && centerX > prevCenterX)
	{
		mainView.setCenter({ centerX, 450.f });
		mParallaxBG.update(mainView);
	}
	prevCenterX = centerX;
	window.setView(mainView);

	window.draw(mParallaxBG);

	tmap->renderMap(window);

	window.draw(*gameObject->sprite());
}