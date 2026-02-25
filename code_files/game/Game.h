#ifndef GAME_H___
#define GAME_H___

class Game
{
	void ProcessInput();
	void UpdateGame();
	void UpdateAnimations();
	void HandleCollisions();
	void RenderFrame();
public:
	bool Initialize();
	void Shutdown();

	void Run();

};
#endif