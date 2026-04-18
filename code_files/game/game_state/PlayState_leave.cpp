#include "PlayState.h"

void PlayState::leaveImpl() {
    std::cout << "Leaving PlayState" << std::endl;

    if (gameObject)
        delete gameObject;

    if (tmap)
    {
        tmap.reset();
    }
}