#pragma once
#include "postfx/PostFxManager.h"
#include <SDL_video.h>
#include "terrain/Heightfield.h"

namespace engine 
{
    class Scene;
    class ModelManager;
    class FlareManager;
    class EnvironmentManager;
    class FboInfo;
}

class Game
{
public:

    void initialize();
    bool update();
    void render();
    void destroy();

private:
    void initGL();
    bool handleEvents();
    void gui();

private:
    engine::ModelManager* m_modelManager;
    engine::FlareManager* m_flareManager;
    engine::EnvironmentManager* m_environmentManager;
    engine::PostFxManager* m_postfxManager;
    engine::FboInfo* m_mainFrameBuffer;
    engine::Scene* m_scene;
    SDL_Window* m_window;
    int m_width;
    int m_height;
    engine::PostFxManager::PostFxTypes m_currentEffect;
    bool m_useLensFlare;
    engine::HeightField m_heightfield;
};