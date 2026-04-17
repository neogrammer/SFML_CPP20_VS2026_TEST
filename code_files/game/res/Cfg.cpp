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
}


void Cfg::initFonts()
{
    fonts.load((int)Fonts::Bubbly, "assets/fonts/faith.ttf");
}