#ifndef PLAYSTATE_H__
#define PLAYSTATE_H__

#include "../GameState.h"
#include <SFML/Window/Keyboard.hpp>
#include <game/game_objects/AnimObj.h>
#include <iostream>
// A specific implementation
class PlayState : public GameState<PlayState> {

    eStateID mPendingState{ eStateID::None };
    GObj* gameObject{ nullptr };
public:

    eStateID updateImpl(float dt);
    void renderImpl(sf::RenderWindow& window); 
    void handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed);

    void enterImpl();
    void leaveImpl();


};
#endif