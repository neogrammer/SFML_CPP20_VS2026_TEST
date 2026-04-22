#include "PlayState.h"

void PlayState::leaveImpl() {
    std::cout << "Leaving PlayState" << std::endl;

    if (player)
    {
        delete player;
        player = nullptr;
    }

    if (tmap)
    {
        tmap.reset();
    }
}
