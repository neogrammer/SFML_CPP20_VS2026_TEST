#include <SFML/Graphics.hpp>

#include <iostream>

#include "code_files/game/Globs.h"
#include "code_files/game/misc/util.h"
#include "code_files/game/res/Cfg.h"
#include "code_files/game/Game.h"
int main()
{
    Cfg::Initialize();

   
    Game mGame{};

    mGame.Initialize();

    mGame.Run();

    mGame.Shutdown();

    return 0;
}

