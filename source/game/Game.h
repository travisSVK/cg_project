#pragma once
#include "postfx/PostFxManager.h"
#include <SDL_video.h>
#include "terrain/Heightfield.h"
#include "gameobjects/QuestManager.h"

namespace engine 
{
    class Scene;
    class ModelManager;
    class FlareManager;
    class EnvironmentManager;
    class FboInfo;
    class CollisionManager;
}

class GameCamera;
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
    bool m_gameCameraActive;
    engine::ModelManager* m_modelManager;
    engine::FlareManager* m_flareManager;
    engine::EnvironmentManager* m_environmentManager;
    engine::PostFxManager* m_postfxManager;
    engine::CollisionManager* m_collisionManager;
    engine::FboInfo* m_mainFrameBuffer;
    engine::Scene* m_scene;
    engine::HeightField m_heightfield;
    GameCamera* m_gameCamera;
    QuestManager m_questManager;
    SDL_Window* m_window;
    int m_width;
    int m_height;
    engine::PostFxManager::PostFxTypes m_currentEffect;
    engine::PostFxManager::PostFxTypes m_currentScreenEffect;
    bool m_useLensFlare;
    bool m_showTeselatedTerrain;
    bool m_showColliders;
};