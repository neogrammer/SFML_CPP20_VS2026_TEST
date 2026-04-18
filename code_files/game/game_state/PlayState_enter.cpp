#include "PlayState.h"

void PlayState::enterImpl() {
    std::cout << "Entered PlayState" << std::endl;

    if (gameObject)
        delete gameObject;

    gameObject = new AnimObj{ Cfg::Textures::PlayerAtlas, {{0,192},{192,192}}, false , {300.f,300.f}, {192.f,192.f}, {0.f,0.f} };
    dynamic_cast<AnimObj*>(gameObject)->loadAnimations("assets/anims/player.anm");
}