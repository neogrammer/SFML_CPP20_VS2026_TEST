#pragma once
#include "GObj.h"
#include <unordered_map>
#include <vector>
#include <array>
#include <unordered_set>
#include <string>

#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <res/Cfg.h>
#include <algorithm>

#include <SFML/Graphics/Texture.hpp>

enum class AnimName
{
	TeleportIn,
	TeleportLand,
	Idle,
	Run,
	LiftOff,
	Rising,
	JumpPeak,
	Falling,
	Landing,
	Shoot,
	RunShoot,
	DashStart,
	Dashing,
	DashEnd,
	WallKick,
	WallLand,
	WallSlide,
	LiftOffShoot,
	RisingShoot,
	JumpPeakShoot,
	FallingShoot,
	LandingShoot,
	DashStartShoot,
	DashingShoot,
	DashEndShoot,
	WallLandShoot,
	WallSlideShoot,
	WallKickShoot,
	None
};

class AnimObj : public GObj
{
	void setBase();
	void setCopyBase();


	std::unordered_map<AnimName, std::vector<std::vector<sf::IntRect>>> frames; // AnimName -> [0]->UniDirectional || rightFacing [1]->LeftFacing -> vector of intrects for the frames in the texture image
	std::unordered_map < AnimName, std::vector<std::vector<sf::Vector2f>>> offsets;
	std::unordered_map < AnimName, std::vector<std::vector<sf::Vector2f>>> sizes;
	std::unordered_map < AnimName, std::vector<std::vector<float>>> delays;
	std::unordered_map < AnimName, float> loopDelays;
	std::unordered_map < AnimName, bool> loopWaits;
	std::unordered_map < AnimName, bool> looping;
	std::unordered_map < AnimName, bool> uniDirectionals;

	std::unordered_map < AnimName, Cfg::Textures> texIDs;

	std::unordered_set<AnimName> anims; // anim names


	AnimName currentAnim;
	uint8_t currentIndex;
	bool playing;

	float loopElapsed, animElapsed;

public:


	void loadAnimation(AnimName nameID_, Cfg::Textures texID_, std::vector<sf::IntRect>& rects_, std::vector<sf::Vector2f>& offsets_, std::vector<sf::Vector2f>& sizes_, std::vector<float>& delays_, bool loadFirstValue = false,
		bool loopWaits_ = false, float loopDelay_ = 0.f, bool looping = true, bool playing = false);


	AnimObj(const std::string& filename);
	AnimObj(Cfg::Textures texID_, sf::IntRect texRect_, bool uniDirectional_ = false, sf::Vector2f position = { 0.f,0.f }, sf::Vector2f size = { 0.f,0.f }, sf::Vector2f offset_ = { 0.f,0.f }, bool isCopy = false);

	void loadAnimations(std::unordered_map<AnimName, Cfg::Textures>& texID_, std::unordered_set<AnimName>& nameID_, AnimName startAnim, std::unordered_map<AnimName, sf::Vector2f>& frameSizes_, std::unordered_map<AnimName, std::vector<sf::Vector2f>>& offsets_, std::unordered_map<AnimName, std::vector<sf::Vector2f>>& sizes_, std::unordered_map<AnimName, std::vector<float>>& delays_, std::unordered_map<AnimName, uint32_t>& startCols_, std::unordered_map<AnimName, sf::Vector2f> startPxls_, std::unordered_map<AnimName, sf::Vector2f> startPxlsLeft_, std::unordered_map<AnimName, uint32_t>& pitches_, std::unordered_map<AnimName, uint32_t>& numFrames_, sf::Vector2f position_ = { 0.f,0.f }, std::unordered_map<AnimName, bool> uniDirectionals_ = std::unordered_map<AnimName, bool>{}, std::unordered_map<AnimName, bool> loopWaits_ = std::unordered_map<AnimName, bool>{}, std::unordered_map<AnimName, float> loopDelays_ = std::unordered_map<AnimName, float>{}, std::unordered_map<AnimName, bool> loopings_ = std::unordered_map<AnimName, bool>{});

	~AnimObj();

	sf::IntRect getCurrentFrame();

	virtual void update(float dt_) override;
	virtual void swapdate() override;
	void loadAnimations(const std::string& filename);
	void animate(float dt);
	void animBfrSwap();

};