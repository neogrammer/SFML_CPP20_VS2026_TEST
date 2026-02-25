#include <Game.h>

bool Game::Initialize()
{

	// Load assets

	return true;
}

void Game::Shutdown()
{

	// Unload Assets
}

void Game::Run()
{
	// run the proper functions to make a game order
}

void Game::ProcessInput()
{
	// Poll devices for input and change flags
}

void Game::UpdateGame()
{
	// Update all objects, considering the flags
}

void Game::UpdateAnimations()
{
	// Animate to match the update
}

void Game::HandleCollisions()
{
	// Detect and Resolve collisions and change animations again along with state, save this for next frame
}

void Game::RenderFrame()
{
	// render the updates and animation reactions for the past frame
}
