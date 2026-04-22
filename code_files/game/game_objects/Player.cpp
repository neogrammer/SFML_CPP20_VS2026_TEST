#include "Player.h"
#include <res/Cfg.h>

Player::Player()
	: AnimObj(Cfg::Textures::PlayerAtlas, { {0,0},{192,192} }, false, { 400.f,400.f }, { 72.f,114.f }, {64.f,42.f})
{

}

Player::~Player()
{

}

const GObj& Player::operator()()
{
	return *this;
}
