#ifndef __GAMESTATE_H___
#define __GAMESTATE_H___
#include <SFML/Graphics.hpp>
#include <game/GObj.h>

enum class eStateID
{
    Splash,
    Title,
    Play,
    Count,
    None,
};

template<typename Derived>
class GameState
{
    
public:
    GameState<Derived>() {}
    ~GameState<Derived>() {}

    // "Interface" methods that call the derived implementation
    eStateID update(float dt) {
        return static_cast<Derived*>(this)->updateImpl(dt);
    }

    void render(sf::RenderWindow& window) {
        static_cast<Derived*>(this)->renderImpl(window);
    }

    void enter() {
        static_cast<Derived*>(this)->enterImpl();
    }

    void leave()
    {
        static_cast<Derived*>(this)->leaveImpl();
    }

    void handleKeyEvent(sf::Keyboard::Key key, bool isPressed) {
        static_cast<Derived*>(this)->handleKeyEventInputImpl(key, isPressed);
    }

    void handleStaticInput(float dt, GObj* gameObject) {
        static_cast<Derived*>(this->handleStaticInputImpl(dt, gameObject));
    }

};
#endif