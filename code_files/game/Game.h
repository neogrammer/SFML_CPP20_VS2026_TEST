#ifndef GAME_H___
#define GAME_H___
#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui-SFML.h"
#include "Globs.h"
#include "GObj.h"
#include <memory>
#include <utility>
#include <variant>
#include <optional>
#include <map>
#include <type_traits>
#include "game_state/StateVar.h"

class Game
{
	ImFont* f0;
	ImFont* f1;
	ImFont* f2;
	ImFont* f3;
	ImFont* f4;
	ImFont* f5;
	ImFont* f6;
	ImFont* f7;
	ImFont* f8;
	ImFont* f9;
	ImFont* f10;
	ImFont* f11;
	ImFont* f12;


	sf::Font bubblyFnt{ "assets/fonts/faith.ttf" };
	sf::Text testTxt{ bubblyFnt };
	sf::Clock mDeltaClock;
	float delta;

	sf::RenderWindow mWindow;
	bool mShouldShutDown{ false };

	void processEvents();
	void update(float dt);
	void handleKeyEvent(sf::Keyboard::Key key, bool isPressed);
	void render();
	void resizeBackground();

	StateVar* currState{nullptr};
	std::map<eStateID, StateVar> stateMap;
	eStateID switchTo{ eStateID::None };

public:

	bool Initialize();
	void Shutdown();
	void Run();


	Game();
	~Game();
};



#endif