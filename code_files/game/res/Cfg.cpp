#include <res/Cfg.h>
#include <Globs.h>

ResourceManager<sf::Texture, int> Cfg::textures = {};
ResourceManager<sf::Font, int> Cfg::fonts = {};


void Cfg::Initialize()
{
    initTextures();
    initFonts();
}


void Cfg::initTextures()
{
    textures.load((int)Textures::SplashBG, "assets/textures/splash_state/GreyBG.png");
    textures.load((int)Textures::TitleBG, "assets/textures/splash_state/BG.png");
    textures.load((int)Textures::BallsBG, "assets/textures/splash_state/SplashBG_Balls.png");
    textures.load((int)Textures::PlayerAtlas, "assets/textures/actors/mmedit.png");
    textures.load((int)Textures::TilesetJungle1, "assets/textures/tileset1.png");
    textures.load((int)Textures::BlankTile, "assets/textures/blank_tile.png");
    textures.load((int)Textures::PLXBG1, "assets/textures/backgrounds/bg1/8.png");
    textures.load((int)Textures::PLXBG2, "assets/textures/backgrounds/bg1/7.png");
    textures.load((int)Textures::PLXBG3, "assets/textures/backgrounds/bg1/6.png");
    textures.load((int)Textures::PLXBG4, "assets/textures/backgrounds/bg1/5.png");
    textures.load((int)Textures::PLXBG5, "assets/textures/backgrounds/bg1/4.png");
    textures.load((int)Textures::PLXBG6, "assets/textures/backgrounds/bg1/3.png");
    textures.load((int)Textures::PLXBG7, "assets/textures/backgrounds/bg1/2.png");
    textures.load((int)Textures::PLXBG8, "assets/textures/backgrounds/bg1/1.png");


}


void Cfg::initFonts()
{
    fonts.load((int)Fonts::Bubbly, "assets/fonts/faith.ttf");
}