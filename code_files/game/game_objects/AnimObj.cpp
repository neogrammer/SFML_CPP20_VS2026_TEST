#include "AnimObj.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

void AnimObj::setBase()
{
	auto& c = *dynamic_cast<AnimObj*>(copy);

	if (!c.anims.contains(c.currentAnim))
	{
		std::cout << "Current Animation is not in the set for this entity" << std::endl;
		return;
	}


	int currDir = (c.isFacingRight()) ? 0 : 1;
	if (c.currentIndex < 0 || c.currentIndex >= c.frames[c.currentAnim].at(currDir).size())
	{
		std::cout << "Current Index out of range in current animation" << std::endl;
		return;
	}


	setOffset(c.offsets[c.currentAnim][currDir][c.currentIndex]);
	setSize(c.sizes[c.currentAnim][currDir][c.currentIndex]);
	
}

void AnimObj::setCopyBase()
{
	auto& c = *dynamic_cast<AnimObj*>(copy);
	if (!c.anims.contains(c.currentAnim))
	{
		std::cout << "Current Animation is not in the set for this entity" << std::endl;
		return;
	}


	int currDir = (c.isFacingRight()) ? 0 : 1;
	if (c.currentIndex < 0 || c.currentIndex >= c.frames[c.currentAnim].at(currDir).size())
	{
		std::cout << "Current Index out of range in current animation" << std::endl;
		return;
	}
	
	copy->setOffset(offsets[c.currentAnim][currDir][c.currentIndex]);
	copy->setSize(sizes[c.currentAnim][currDir][c.currentIndex]);
}

AnimObj::AnimObj(const std::string& filename)
	: GObj{}
{
	loadAnimations(filename);
}

void AnimObj::loadAnimation(AnimName nameID_, Cfg::Textures texID_, std::vector<sf::IntRect>& rects_, std::vector<sf::Vector2f>& offsets_, std::vector<sf::Vector2f>& sizes_, std::vector<float>& delays_, bool loadFirstValue, bool loopWaits_, float loopDelay_, bool looping_, bool playing_)
{
	if (anims.contains(nameID_))
	{
		std::cout << "Animation already exists! Bouncing back." << std::endl;
		return;
	}

	anims.emplace(nameID_);
	frames[nameID_] = std::vector<std::vector<sf::IntRect>>{};
	offsets[nameID_] = std::vector<std::vector<sf::Vector2f>>{};
	sizes[nameID_] = std::vector<std::vector<sf::Vector2f>>{};
	delays[nameID_] = std::vector<std::vector<float>>{};




	frames[nameID_].push_back(std::vector<sf::IntRect>{});
	frames[nameID_][0].clear();
	if (this->isUniDirectional())
	{
		frames[nameID_][0].reserve(rects_.size());
		for (int i = 0; i < rects_.size(); i++)
			frames[nameID_][0].emplace_back(rects_[i]);
	}
	else
	{
		frames[nameID_][0].reserve(rects_.size() / 2);
		for (size_t i = 0; i < rects_.size() / 2; i++)
			frames[nameID_][0].emplace_back(rects_[i]);
	}

	if (!this->isUniDirectional())
	{
		frames[nameID_].push_back(std::vector<sf::IntRect>{});
		frames[nameID_][1].clear();
		frames[nameID_][1].reserve(rects_.size() / 2);
		for (size_t i = rects_.size() / 2; i < rects_.size(); i++)
			frames[nameID_][1].emplace_back(rects_[i]);
	}


	sizes[nameID_].push_back(std::vector<sf::Vector2f>{});
	sizes[nameID_][0].clear();
	if (this->isUniDirectional())
	{
		sizes[nameID_][0].reserve(sizes_.size());
		for (size_t i = 0; i < sizes_.size(); i++)
			sizes[nameID_][0].emplace_back(sizes_[i]);
	}
	else
	{
		sizes[nameID_][0].reserve(sizes_.size() / 2);
		for (size_t i = 0; i < sizes_.size() / 2; i++)
			sizes[nameID_][0].emplace_back(sizes_[i]);
		sizes[nameID_].push_back(std::vector<sf::Vector2f>{});
		sizes[nameID_][1].clear();
		sizes[nameID_][1].reserve(sizes_.size() / 2);
		for (size_t i = sizes_.size() / 2; i < sizes_.size(); i++)
			sizes[nameID_][1].emplace_back(sizes_[i]);
	}

	offsets[nameID_].push_back(std::vector<sf::Vector2f>{});
	offsets[nameID_][0].clear();
	if (this->isUniDirectional())
	{
		offsets[nameID_][0].reserve(offsets_.size());
		for (size_t i = 0; i < offsets_.size(); i++)
			offsets[nameID_][0].emplace_back(offsets_[i]);
	}
	else
	{
		offsets[nameID_][0].reserve(offsets_.size() / 2);
		for (int i = 0; i < offsets_.size() / 2; i++)
			offsets[nameID_][0].emplace_back(offsets_[i]);
		offsets[nameID_].push_back(std::vector<sf::Vector2f>{});
		offsets[nameID_][1].clear();
		offsets[nameID_][1].reserve(offsets_.size() / 2);
		for (size_t i = offsets_.size() / 2; i < offsets_.size(); i++)
			offsets[nameID_][1].emplace_back(offsets_[i]);
	}




	delays[nameID_].push_back(std::vector<float>{});
	delays[nameID_][0].clear();
	if (this->isUniDirectional())
	{
		delays[nameID_][0].reserve(delays_.size());
		for (int i = 0; i < delays_.size(); i++)
			delays[nameID_][0].emplace_back(delays_[i]);
	}
	else
	{
		delays[nameID_][0].reserve(delays_.size() / 2);
		for (int i = 0; i < delays_.size() / 2; i++)
			delays[nameID_][0].emplace_back(delays_[i]);
		delays[nameID_].push_back(std::vector<float>{});
		delays[nameID_][1].clear();
		delays[nameID_][1].reserve(delays_.size() / 2);
		for (size_t i = delays_.size() / 2; i < delays_.size(); i++)
			delays[nameID_][1].emplace_back(delays_[i]);
	}



	if (loopDelays.find(nameID_) == loopDelays.end())
		loopDelays[nameID_] = loopDelay_;

	if (looping.find(nameID_) == looping.end())
		looping[nameID_] = looping_;

	if (loopWaits.find(nameID_) == loopWaits.end())
		loopWaits[nameID_] = loopWaits_;

	if (texIDs.find(nameID_) == texIDs.end())
	{
		texIDs[nameID_] = texID_;
	}

	playing = true;
	loopElapsed = 0.f;
	animElapsed = 0.f;



	if (loadFirstValue)
	{
		currentAnim = nameID_;
		currentIndex = 0;
		setFacingRight(true);
		setID(texIDs[nameID_]);
	}



}

AnimObj::AnimObj(Cfg::Textures texID_, sf::IntRect texRect_, bool uniDirectional_, sf::Vector2f position_, sf::Vector2f size_, sf::Vector2f offset_, bool isCopy)
	: GObj{ texID_, texRect_,uniDirectional_, position_, size_, offset_ }
{

	if (!isCopy)
	{
		if (copy)
		{
			delete copy;
		}

		copy = new AnimObj{ texID_, texRect_, uniDirectional_, position_, size_, offset_, true };
	}

}

void AnimObj::loadAnimations(std::unordered_map<AnimName, Cfg::Textures>& texID_, std::unordered_set<AnimName>& nameID_, AnimName startAnim, std::unordered_map<AnimName, sf::Vector2f>& frameSizes_,
	std::unordered_map<AnimName, std::vector<sf::Vector2f>>& offsets_, std::unordered_map<AnimName, std::vector<sf::Vector2f>>& sizes_,
	std::unordered_map<AnimName, std::vector<float>>& delays_, std::unordered_map<AnimName, uint32_t>& startCols_,
	std::unordered_map<AnimName, sf::Vector2f> startPxls_, std::unordered_map<AnimName, sf::Vector2f> startPxlsLeft_, std::unordered_map<AnimName, uint32_t>& pitches_, std::unordered_map<AnimName, uint32_t>& numFrames_,
	sf::Vector2f position_, std::unordered_map < AnimName, bool> uniDirectionals_, std::unordered_map<AnimName, bool> loopWaits_, std::unordered_map<AnimName, float> loopDelays_, std::unordered_map<AnimName, bool> loopings_)
{
	animElapsed = 0.f;
	currentAnim = AnimName::None;
	currentIndex = 0;
	loopElapsed = 0.f;
	playing = true;

	AnimObj* other = dynamic_cast<AnimObj*>(copy);

	other->animElapsed = 0.f;
	other->currentAnim = AnimName::None;
	other->currentIndex = 0;
	other->loopElapsed = 0.f;
	other->playing = true;


	for (auto& aname : nameID_)
	{
		// For each animation, setup
		// 

		// Texture IDs and other minor parameters
		anims.emplace(aname);
		const bool uni = uniDirectionals_.at(aname);
		const bool lw = loopWaits_.at(aname);
		const float ld = loopDelays_.at(aname);
		const bool lp = loopings_.at(aname);
		const Cfg::Textures tID = texID_.at(aname);
		texIDs[aname] = tID;
		loopDelays[aname] = ld;
		loopWaits[aname] = lw;
		looping[aname] = lp;


		other->anims.emplace(aname);
		other->texIDs[aname] = tID;
		other->loopDelays[aname] = ld;
		other->loopWaits[aname] = lw;
		other->looping[aname] = lp;

		// major params

		// Texture Offsets
		offsets[aname] = std::vector<std::vector<sf::Vector2f>>{};
		offsets.at(aname).clear();
		const auto& off = offsets_.at(aname);
		if (uni)
		{
			offsets.at(aname).reserve(1);
			offsets.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			offsets.at(aname).back().clear();
			offsets.at(aname).back().reserve(off.size());
			for (int j = 0; j < off.size(); j++)
				offsets.at(aname)[0].emplace_back(off[j]);
		}
		else
		{
			offsets.at(aname).reserve(2);
			offsets.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			offsets.at(aname)[0].clear();
			offsets.at(aname)[0].reserve(off.size() / 2);
			offsets.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			offsets.at(aname)[1].clear();
			offsets.at(aname)[1].reserve(off.size() / 2);
			for (int j = 0; j < (int)(off.size() / 2); j++)
				offsets.at(aname)[0].emplace_back(off[j]);
			for (int j = (int)(off.size() / 2); j < (int)(off.size()); j++)
				offsets.at(aname)[1].emplace_back(off[j]);
		}


		other->offsets[aname] = std::vector<std::vector<sf::Vector2f>>{};
		other->offsets.at(aname).clear();
		
		if (uni)
		{
			other->offsets.at(aname).reserve(1);
			other->offsets.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			other->offsets.at(aname).back().clear();
			other->offsets.at(aname).back().reserve(off.size());
			for (int j = 0; j < off.size(); j++)
				other->offsets.at(aname)[0].emplace_back(off[j]);
		}
		else
		{
			other->offsets.at(aname).reserve(2);
			other->offsets.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			other->offsets.at(aname)[0].clear();
			other->offsets.at(aname)[0].reserve(off.size() / 2);
			other->offsets.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			other->offsets.at(aname)[1].clear();
			other->offsets.at(aname)[1].reserve(off.size() / 2);
			for (int j = 0; j < (int)(off.size() / 2); j++)
				other->offsets.at(aname)[0].emplace_back(off[j]);
			for (int j = (int)(off.size() / 2); j < (int)(off.size()); j++)
				other->offsets.at(aname)[1].emplace_back(off[j]);
		}


		// Texture IntRects
		// startPxl is where to start for that animation in the texture in the minimum top left of both x and y, then startCol is the start column of the mini sheet,
		//  which stretches out map<AnimName,FrameSize> sizes * map<AnimName,uint8_t pitch from the startPxl
		//  This give you an inner sprite sheet, and you can have any dimensions and sizes you want, for giant texture atlases
		float frWidth = frameSizes_.at(aname).x;
		float frHeight = frameSizes_.at(aname).y;
		auto stPxl = startPxls_.at(aname);
		auto stPxlLeft = startPxlsLeft_.at(aname);
		auto stCol = startCols_.at(aname);
		auto pitch = pitches_.at(aname);
		auto numFrames = numFrames_.at(aname);
		const auto& size = sizes_.at(aname);
		const auto& delay = delays_.at(aname);

		const size_t expect = uni ? (size_t)numFrames : (size_t)numFrames * 2;
		if (off.size() != expect || size.size() != expect || delay.size() != expect)
		{
			throw std::runtime_error("Anim list sizes mismatch for anim");
		}

		uint32_t numRows = (numFrames + pitch - 1) / pitch;

		if (uni)
		{
			frames[aname] = std::vector<std::vector<sf::IntRect>>{};
			auto& frame = frames.at(aname);
			frame.clear();
			frame.reserve(1);
			frame.emplace_back(std::vector<sf::IntRect>{});
			frame[0].clear();
			frame[0].reserve(off.size());
			uint32_t counter = 0Ui32;
			for (int k = 0; k < (int)off.size() && (k + stCol) * frWidth + stPxl.x < stPxl.x + (pitch * frWidth); k++)
			{

				frame[0].emplace_back(sf::IntRect{ {(int)(stPxl.x + (k + stCol) * frWidth),(int)stPxl.y}, {(int)frWidth,(int)frHeight} });
				counter++;
			}
			if (counter < numFrames)
			{
				for (uint32_t z = 1; z < numRows; z++)
				{
					for (uint32_t k = 0; k < pitch && counter < numFrames; k++)
					{
						frame[0].emplace_back(sf::IntRect{ {(int)(stPxl.x + (k * frWidth)),(int)(stPxl.y + (z * frHeight))}, {(int)frWidth,(int)frHeight} });
						counter++;
					}
				}
			}

			other->frames[aname] = std::vector<std::vector<sf::IntRect>>{};
			auto& frameCpy = other->frames.at(aname);
			frameCpy.clear();
			frameCpy.reserve(1);
			frameCpy.emplace_back(std::vector<sf::IntRect>{});
			frameCpy[0].clear();
			frameCpy[0].reserve(off.size());
			uint32_t counterCpy = 0Ui32;
			for (int k = 0; k < (int)off.size() && (k + stCol) * frWidth + stPxl.x < stPxl.x + (pitch * frWidth); k++)
			{

				frameCpy[0].emplace_back(sf::IntRect{ {(int)(stPxl.x + (k + stCol) * frWidth),(int)stPxl.y}, {(int)frWidth,(int)frHeight} });
				counterCpy++;
			}
			if (counterCpy < numFrames)
			{
				for (uint32_t z = 1; z < numRows; z++)
				{
					for (uint32_t k = 0; k < pitch && counterCpy < numFrames; k++)
					{
						frameCpy[0].emplace_back(sf::IntRect{ {(int)(stPxl.x + (k * frWidth)),(int)(stPxl.y + (z * frHeight))}, {(int)frWidth,(int)frHeight} });
						counterCpy++;
					}
				}
			}

		}
		else
		{

			frames[aname] = std::vector<std::vector<sf::IntRect>>{};
			auto& frame = frames.at(aname);
			frame.clear();
			frame.reserve(2);
			frame.emplace_back(std::vector<sf::IntRect>{});
			frame[0].clear();
			frame[0].reserve(off.size() / 2);
			frame.emplace_back(std::vector<sf::IntRect>{});
			frame[1].clear();
			frame[1].reserve(off.size() / 2);
			uint32_t counter = 0Ui32;
			for (int k = 0; k < (int)off.size() / 2 && (k + stCol) * frWidth + stPxl.x < stPxl.x + (pitch * frWidth); k++)
			{
				frame[0].emplace_back(sf::IntRect{ {(int)(stPxl.x + (k + stCol) * frWidth),(int)stPxl.y}, {(int)frWidth,(int)frHeight} });
				counter++;
			}
			if (counter < numFrames)
			{
				for (uint32_t z = 1; z < numRows; z++)
				{
					for (uint32_t k = 0; k < pitch && counter < numFrames; k++)
					{
						frame[0].emplace_back(sf::IntRect{ {(int)(stPxl.x + (k * frWidth)),(int)(stPxl.y + (z * frHeight))}, {(int)frWidth,(int)frHeight} });
						counter++;
					}
				}
			}
			counter = 0Ui32;
			for (int k = 0; k < (int)off.size() / 2 && (k + stCol) * frWidth + stPxlLeft.x < stPxlLeft.x + (pitch * frWidth); k++)
			{
				frame[1].emplace_back(sf::IntRect{ {(int)(stPxlLeft.x + (k + stCol) * frWidth),(int)stPxlLeft.y}, {(int)frWidth,(int)frHeight} });
				counter++;
			}
			if (counter < numFrames)
			{
				for (uint32_t z = 1; z < numRows; z++)
				{
					for (uint32_t k = 0; k < pitch && counter < numFrames; k++)
					{
						frame[1].emplace_back(sf::IntRect{ {(int)(stPxlLeft.x + (k * frWidth)),(int)(stPxlLeft.y + (z * frHeight))}, {(int)frWidth,(int)frHeight} });
						counter++;
					}
				}
			}


			other->frames[aname] = std::vector<std::vector<sf::IntRect>>{};
			auto& frameCpy = other->frames.at(aname);
			frameCpy.clear();
			frameCpy.reserve(2);
			frameCpy.emplace_back(std::vector<sf::IntRect>{});
			frameCpy[0].clear();
			frameCpy[0].reserve(off.size() / 2);
			frameCpy.emplace_back(std::vector<sf::IntRect>{});
			frameCpy[1].clear();
			frameCpy[1].reserve(off.size() / 2);
			uint32_t counterCpy = 0Ui32;
			for (int k = 0; k < (int)off.size() / 2 && (k + stCol) * frWidth + stPxl.x < stPxl.x + (pitch * frWidth); k++)
			{
				frameCpy[0].emplace_back(sf::IntRect{ {(int)(stPxl.x + (k + stCol) * frWidth),(int)stPxl.y}, {(int)frWidth,(int)frHeight} });
				counterCpy++;
			}
			if (counterCpy < numFrames)
			{
				for (uint32_t z = 1; z < numRows; z++)
				{
					for (uint32_t k = 0; k < pitch && counterCpy < numFrames; k++)
					{
						frameCpy[0].emplace_back(sf::IntRect{ {(int)(stPxl.x + (k * frWidth)),(int)(stPxl.y + (z * frHeight))}, {(int)frWidth,(int)frHeight} });
						counterCpy++;
					}
				}
			}
			counterCpy = 0Ui32;
			for (int k = 0; k < (int)off.size() / 2 && (k + stCol) * frWidth + stPxlLeft.x < stPxlLeft.x + (pitch * frWidth); k++)
			{
				frameCpy[1].emplace_back(sf::IntRect{ {(int)(stPxlLeft.x + (k + stCol) * frWidth),(int)stPxlLeft.y}, {(int)frWidth,(int)frHeight} });
				counterCpy++;
			}
			if (counterCpy < numFrames)
			{
				for (uint32_t z = 1; z < numRows; z++)
				{
					for (uint32_t k = 0; k < pitch && counterCpy < numFrames; k++)
					{
						frameCpy[1].emplace_back(sf::IntRect{ {(int)(stPxlLeft.x + (k * frWidth)),(int)(stPxlLeft.y + (z * frHeight))}, {(int)frWidth,(int)frHeight} });
						counterCpy++;
					}
				}
			}

		}


		// ---------------- END OF TEXTURE RECTS

		//  Gameplay Sizes
				// Texture Offsets
		sizes[aname] = std::vector<std::vector<sf::Vector2f>>{};
		sizes.at(aname).clear();

		if (uni)
		{
			sizes.at(aname).reserve(1);
			sizes.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			sizes.at(aname).back().clear();
			sizes.at(aname).back().reserve(size.size());
			for (int j = 0; j < size.size(); j++)
				sizes.at(aname)[0].emplace_back(size[j]);
		}
		else
		{
			sizes.at(aname).reserve(2);
			sizes.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			sizes.at(aname)[0].clear();
			sizes.at(aname)[0].reserve(size.size() / 2);
			sizes.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			sizes.at(aname)[1].clear();
			sizes.at(aname)[1].reserve(size.size() / 2);
			for (int j = 0; j < size.size() / 2; j++)
				sizes.at(aname)[0].emplace_back(size[j]);
			for (int j = (int)size.size() / 2; j < (int)size.size(); j++)
				sizes.at(aname)[1].emplace_back(size[j]);
		}

		other->sizes[aname] = std::vector<std::vector<sf::Vector2f>>{};
		other->sizes.at(aname).clear();

		if (uni)
		{
			other->sizes.at(aname).reserve(1);
			other->sizes.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			other->sizes.at(aname).back().clear();
			other->sizes.at(aname).back().reserve(size.size());
			for (int j = 0; j < size.size(); j++)
				other->sizes.at(aname)[0].emplace_back(size[j]);
		}
		else
		{
			other->sizes.at(aname).reserve(2);
			other->sizes.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			other->sizes.at(aname)[0].clear();
			other->sizes.at(aname)[0].reserve(size.size() / 2);
			other->sizes.at(aname).emplace_back(std::vector<sf::Vector2f>{});
			other->sizes.at(aname)[1].clear();
			other->sizes.at(aname)[1].reserve(size.size() / 2);
			for (int j = 0; j < size.size() / 2; j++)
				other->sizes.at(aname)[0].emplace_back(size[j]);
			for (int j = (int)size.size() / 2; j < (int)size.size(); j++)
				other->sizes.at(aname)[1].emplace_back(size[j]);
		}


		// End of Gameplay Sizes

		//  Animation Frame Delays
				// Texture Offsets
		delays[aname] = std::vector<std::vector<float>>{};
		delays.at(aname).clear();

		if (uni)
		{
			delays.at(aname).reserve(1);
			delays.at(aname).emplace_back(std::vector<float>{});
			delays.at(aname).back().clear();
			delays.at(aname).back().reserve(delay.size());
			for (int j = 0; j < delay.size(); j++)
				delays.at(aname)[0].emplace_back(delay[j] / 1000.f);
		}
		else
		{
			delays.at(aname).reserve(2);
			delays.at(aname).emplace_back(std::vector < float > {});
			delays.at(aname)[0].clear();
			delays.at(aname)[0].reserve(delay.size() / 2);
			delays.at(aname).emplace_back(std::vector<float>{});
			delays.at(aname)[1].clear();
			delays.at(aname)[1].reserve(delay.size() / 2);
			for (int j = 0; j < delay.size() / 2; j++)
				delays.at(aname)[0].emplace_back(delay[j] / 1000.f);
			for (int j = (int)delay.size() / 2; j < (int)delay.size(); j++)
				delays.at(aname)[1].emplace_back(delay[j] / 1000.f);
		}

		other->delays[aname] = std::vector<std::vector<float>>{};
		other->delays.at(aname).clear();

		if (uni)
		{
			other->delays.at(aname).reserve(1);
			other->delays.at(aname).emplace_back(std::vector<float>{});
			other->delays.at(aname).back().clear();
			other->delays.at(aname).back().reserve(delay.size());
			for (int j = 0; j < delay.size(); j++)
				other->delays.at(aname)[0].emplace_back(delay[j] / 1000.f);
		}
		else
		{
			other->delays.at(aname).reserve(2);
			other->delays.at(aname).emplace_back(std::vector < float > {});
			other->delays.at(aname)[0].clear();
			other->delays.at(aname)[0].reserve(delay.size() / 2);
			other->delays.at(aname).emplace_back(std::vector<float>{});
			other->delays.at(aname)[1].clear();
			other->delays.at(aname)[1].reserve(delay.size() / 2);
			for (int j = 0; j < delay.size() / 2; j++)
				other->delays.at(aname)[0].emplace_back(delay[j] / 1000.f);
			for (int j = (int)delay.size() / 2; j < (int)delay.size(); j++)
				other->delays.at(aname)[1].emplace_back(delay[j] / 1000.f);
		}


		// End of Animation Frame Delays
	}
	if (!nameID_.contains(startAnim))
	{
		std::cout << "Shit, startAnim is not one of the animations!" << std::endl;
		return;
	}
	currentAnim = startAnim;
	other->currentAnim = startAnim;
	setUniDirectional(uniDirectionals_.at(startAnim));
	other->setUniDirectional(uniDirectionals_.at(startAnim));
	setCopyBase();
	setBase();
}



AnimObj::~AnimObj()
{
}

sf::IntRect AnimObj::getCurrentFrame()
{
	int currDir = (this->isFacingRight()) ? 0 : 1;
	return frames.at(currentAnim).at(currDir).at(currentIndex);
}

void AnimObj::update(float dt_)
{
	

	if (copy == nullptr)
	{
		std::cout << "anim obj copy not good!" << std::endl;
		throw std::runtime_error("anim obj copy not good!");
	}

	// in this order double buffer
	GObj::update(dt_);
	animate(dt_);
	setCopyBase();
}

void AnimObj::swapdate()
{
	animBfrSwap();
	GObj::swapdate();
	setBase();
}

// Assumes: AnimName and TextureID are enums (or constructible from int)

void AnimObj::loadAnimations(const std::string& filename)
{
	std::ifstream in(filename);
	if (!in.is_open())
		throw std::runtime_error("AnimObj::loadAnimations: failed to open file: " + filename);

	// Containers required by the monster
	std::unordered_map<AnimName, Cfg::Textures> texID;
	std::unordered_set<AnimName> nameID;
	std::unordered_map<AnimName, sf::Vector2f> frameSizes;
	std::unordered_map<AnimName, std::vector<sf::Vector2f>> offsets;
	std::unordered_map<AnimName, std::vector<sf::Vector2f>> sizes;
	std::unordered_map<AnimName, std::vector<float>> delays;
	std::unordered_map<AnimName, uint32_t> startCols;
	std::unordered_map<AnimName, sf::Vector2f> startPxls;
	std::unordered_map<AnimName, sf::Vector2f> startPxlsLeft;
	std::unordered_map<AnimName, uint32_t> pitches;
	std::unordered_map<AnimName, uint32_t> numFrames;
	std::unordered_map<AnimName, bool> uniDirectionals;
	std::unordered_map<AnimName, bool> loopWaits;
	std::unordered_map<AnimName, float> loopDelays;
	std::unordered_map<AnimName, bool> loopings;

	// 1) position
	sf::Vector2f position{};
	in >> position.x >> position.y;

	// 2) startAnim
	int startAnimInt = 0;
	in >> startAnimInt;
	const AnimName startAnim = static_cast<AnimName>(startAnimInt);

	// 3) animCount
	int animCount = 0;
	in >> animCount;

	// 4) read each anim block
	for (int i = 0; i < animCount; ++i)
	{
		if (i == 14)
		{
			int h = 0;
		}

		// 4.1 header
		int animNameInt = 0;
		int texIdInt = 0;
		int uniInt = 0;
		int loopWaitInt = 0;
		float loopDelay = 0.f;
		int loopingInt = 0;

		in >> animNameInt >> texIdInt >> uniInt >> loopWaitInt >> loopDelay >> loopingInt;

		const AnimName aname = static_cast<AnimName>(animNameInt);
		const Cfg::Textures tid = static_cast<Cfg::Textures>(texIdInt);
		const bool uni = (uniInt != 0);
		const bool lw = (loopWaitInt != 0);
		const bool lp = (loopingInt != 0);

		nameID.emplace(aname);
		texID[aname] = tid;
		uniDirectionals[aname] = uni;
		loopWaits[aname] = lw;
		loopDelays[aname] = loopDelay;
		loopings[aname] = lp;

		// 4.2 frame size
		sf::Vector2f fr{};
		in >> fr.x >> fr.y;
		frameSizes[aname] = fr;

		// 4.3 startPxl right
		sf::Vector2f st{};
		in >> st.x >> st.y;
		startPxls[aname] = st;

		// 4.4 startPxl left (still required even if uni; can be same as right)
		sf::Vector2f stL{};
		in >> stL.x >> stL.y;
		startPxlsLeft[aname] = stL;

		// 4.5 atlas params
		uint32_t stCol = 0;
		uint32_t pitch = 0;
		uint32_t nframes = 0;
		in >> stCol >> pitch >> nframes;

		startCols[aname] = stCol;
		pitches[aname] = pitch;
		numFrames[aname] = nframes;

		// expected count per list
		const uint32_t expect = uni ? nframes : (nframes * 2);

		// 4.6 OFFSETS (expect Vector2f)
		offsets[aname].clear();
		offsets[aname].reserve(expect);
		for (uint32_t j = 0; j < expect; ++j)
		{
			sf::Vector2f v{};
			in >> v.x >> v.y;
			offsets[aname].push_back(v);
		}

		// 4.7 SIZES (expect Vector2f)
		sizes[aname].clear();
		sizes[aname].reserve(expect);
		for (uint32_t j = 0; j < expect; ++j)
		{
			sf::Vector2f v{};
			in >> v.x >> v.y;
			sizes[aname].push_back(v);
		}

		// 4.8 DELAYS (expect float)
		delays[aname].clear();
		delays[aname].reserve(expect);
		for (uint32_t j = 0; j < expect; ++j)
		{
			float d = 0.f;
			in >> d;
			delays[aname].push_back(d);
		}
		if (i == 27)
		{
			int h = 0;
		}
	}

	// Call the monster
	loadAnimations(
		texID,
		nameID,
		startAnim,
		frameSizes,
		offsets,
		sizes,
		delays,
		startCols,
		startPxls,
		startPxlsLeft,
		pitches,
		numFrames,
		position,
		uniDirectionals,
		loopWaits,
		loopDelays,
		loopings
	);

	currentAnim = AnimName::Idle;
	setCopyBase();
	setBase();
}

void AnimObj::animate(float dt)
{
	auto& c = *dynamic_cast<AnimObj*>(copy);
	c.animElapsed += dt;
	int currDir = (c.isFacingRight()) ? 0 : 1;
	if (c.currentIndex < 0 || c.currentIndex >= frames[c.currentAnim].at(currDir).size())
	{
		std::cout << "Current Index out of range in current animation" << std::endl;
		return;
	}
	if (c.animElapsed >= delays[c.currentAnim][currDir][c.currentIndex])
	{

		c.animElapsed = 0.f;
		c.currentIndex++;
		if (c.currentIndex >= frames[c.currentAnim].at(currDir).size())
		{
			c.currentIndex = 0;
		}
	}

	GObj::setRectCpy(c.frames[c.currentAnim].at(currDir).at(c.currentIndex));

}

void AnimObj::animBfrSwap()
{
	auto& c = *dynamic_cast<AnimObj*>(copy);
	
	loopElapsed = c.loopElapsed;
	playing = c.playing;;
	animElapsed = c.animElapsed;
	currentIndex = c.currentIndex;
	currentAnim = c.currentAnim;

	GObj::setRect(c.getRect());
}

AnimName AnimObj::getCurrentAnim()
{
	return currentAnim;
}

void AnimObj::setCurrentAnim(AnimName anim)
{
	auto& c = *dynamic_cast<AnimObj*>(copy);

	c.currentAnim = anim;
	c.currentIndex = 0;
}
