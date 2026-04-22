#include "PlayState.h"

void PlayState::handleKeyEventInputImpl(sf::Keyboard::Key key, bool isPressed)
{
    (void)key;
    (void)isPressed;

    // Movement is sampled once per update in handleStaticInputImpl().
    // Keeping key events out of velocity/animation prevents release events
    // from fighting the live -> copy -> collision -> swap frame order.
}
