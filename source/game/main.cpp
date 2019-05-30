#include "Game.h"

int main(int argc, char *argv[])
{
    Game game;
    auto startTime = std::chrono::system_clock::now();
    float currentTime = 0.0f;
    float previousTime = 0.0f;
    float deltaTime = 0.0f;
    game.initialize();
	bool stopRendering = false;

	while (!stopRendering) {
        std::chrono::duration<float> timeSinceStart = std::chrono::system_clock::now() - startTime;
        previousTime = currentTime;
        currentTime = timeSinceStart.count();
        deltaTime = currentTime - previousTime;
        game.render(deltaTime);
        stopRendering = game.update();
	}

    game.destroy();
	return 0;
}
