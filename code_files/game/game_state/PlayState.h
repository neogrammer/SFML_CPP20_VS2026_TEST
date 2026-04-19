#ifndef PLAYSTATE_H__
#define PLAYSTATE_H__

#include "../GameState.h"
#include <SFML/Window/Keyboard.hpp>
#include <game/game_objects/AnimObj.h>
#include <iostream>
#include "../../game/map/Tilemap.h"
#include <memory>
#include "../map/ParallaxBG.h"

// A specific implementation
class PlayState : public GameState<PlayState> {

    std::unique_ptr<Tilemap> tmap{};
    ParallaxBG mParallaxBG{ 8 };
    eStateID mPendingState{ eStateID::None };
    GObj* gameObject{ nullptr };
public:
    PlayState(sf::RenderWindow& wnd);
    eStateID updateImpl(float dt);
    void renderImpl(sf::RenderWindow& window); 
    void handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed);
    void handleStaticInputImpl(float dt, GObj* gameObject);
    void enterImpl();
    void leaveImpl();


};
#endif