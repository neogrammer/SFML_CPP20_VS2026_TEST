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
}


void Cfg::initFonts()
{
    fonts.load((int)Fonts::Bubbly, "assets/fonts/faith.ttf");
}