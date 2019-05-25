#include "Game.h"

int main(int argc, char *argv[])
{
    Game game;
    game.initialize();
	bool stopRendering = false;

	while (!stopRendering) {
        game.render();
        stopRendering = game.update();
	}

    game.destroy();
	return 0;
}
