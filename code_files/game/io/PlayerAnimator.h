#pragma once
class Player;
class GObj;
#include <game/io/ActionMgr.h>

class PlayerAnimator
{
	Player* player{ nullptr };
	AnimName nextState{ AnimName::None };

	ActionMgr actionMgr{};

public:

	
	// check button states
	void evalContext();

	bool shouldChangeAnim{ false };

	// clear nextState returning it as well as set should change anim to false again
	AnimName getNextAnimAndClear();

	// switch to animation passed in directly
	void switchAnimation(AnimName anim);


	
};