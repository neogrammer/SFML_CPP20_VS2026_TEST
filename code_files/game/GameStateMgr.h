#ifndef __GAMESTATEMGR_H___
#define __GAMESTATEMGR_H___
#include <concepts>
#include <type_traits>
#include <iostream>

// Concept: checks if T is derived from Base (or the same type)
template <typename T, class GameState>
concept DerivedFromBase = std::is_base_of_v<Base, T>;

template <typename T>
class GameStateMgr
{

};

#endif