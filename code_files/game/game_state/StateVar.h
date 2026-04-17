#ifndef STATEVAR_H__
#define STATEVAR_H__

#include <variant>
#include <optional>
#include "PlayState.h"
#include "SplashState.h"
#include "TitleState.h"

using StateVar = std::variant<SplashState, TitleState, PlayState>;

#endif