#ifndef GAME_H___
#define GAME_H___
#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "Globs.h"
#include "Entity.h"
#include <memory>
#include <utility>

class Game
{
	bool mShouldShutDown{ false };

	void processEvents();
	void update(sf::Time deltaTime);
	void handleKeyEvent(sf::Keyboard::Key key, bool isPressed);
	void render();
	void resizeBackground();


	sf::RenderWindow mWindow;
	sf::Texture backgroundTexture;
	sf::Sprite backgroundSprite;

	sptent player;
	sptent ball;
	sptent opponentPaddle;



public:
	bool Initialize();
	void Shutdown();
	void Run();

};
#endif