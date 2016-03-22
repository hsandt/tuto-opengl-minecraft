#include <random>

#include "Game.h"

// GLOBAL
// start random engine with seed
std::mt19937 random_engine(100);

/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main(int argc, char* argv[])
{
	LogConsole::createInstance();

	Game::Instance().Init(argc, argv);
	// main loop run inside init with glut

	// singleton is not created with new but is function scope static, cannot be destroyed, so clean up instead
	Game::Instance().Clean();

	return 0;
}

