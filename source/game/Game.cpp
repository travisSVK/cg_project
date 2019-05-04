#include "Game.h"

// externals
#include <SDL.h>
#include <stb_image.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

// engine specific includes
#include "model/Model.h"
#include "lensFlare/FlareManager.h"
#include "model/ModelManager.h"
#include "environment/EnvironmentManager.h"
#include "postfx/PostFxManager.h"
#include "FboInfo.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "Helper.h"

// game spacific includes
#include "Enums.h"


void Game::initialize()
{
    m_window = engine::init_window_SDL("Game");

    m_modelManager = new engine::ModelManager();
    m_environmentManager = new engine::EnvironmentManager(1.5f);
    
    // setup environment
    const int roughnesses = 8;
    std::vector<std::string> filenames;
    for (int i = 0; i < roughnesses; i++)
    {
        filenames.push_back("../scenes/envmaps/001_dl_" + std::to_string(i) + ".hdr");
    }
    m_environmentManager->createReflectionTexture(filenames);
    m_environmentManager->createEnvironmentTexture("../scenes/envmaps/001.hdr", engine::EnvironmentManager::EnvironmentTextureType::EnvironmentMap);
    m_environmentManager->createEnvironmentTexture("../scenes/envmaps/001_irradiance.hdr", engine::EnvironmentManager::EnvironmentTextureType::IrradianceMap);
    m_environmentManager->createEnvironmentProgram("shaders/background.vert", "shaders/background.frag");

    // TODO this one is for pseudo lens flare
    //m_environmentManager->createEnvironmentTexture("../scenes/ghost_color_gradient.psd", engine::EnvironmentManager::EnvironmentTextureType::GradientTexture);
    
    m_scene = new engine::Scene(glm::vec3(-70.0f, 50.0f, 70.0f), glm::normalize(glm::vec3(0.0f) - glm::vec3(-70.0f, 50.0f, 70.0f)));
    // Load some models.
    glm::mat4 modelMatrix = glm::translate(15.0f * m_scene->getCamera()->getWorldUp());
    m_modelManager->createModel("../scenes/NewShip.obj", static_cast<unsigned int>(GameModels::FighterModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::FighterModel)));
    m_currentEffect = engine::PostFxManager::PostFxTypes::None;
    m_useLensFlare = false;
    m_heightfield.generateMesh(8);
    m_heightfield.loadHeightField("../scenes/nlsFinland/terrainHeightmap.png");
    m_heightfield.loadDiffuseTexture("../scenes/nlsFinland/L3123F_downscaled.jpg");
    initGL();
}

void Game::initGL()
{
    // enable Z-buffering
    glEnable(GL_DEPTH_TEST);

    // enable backface culling
    glEnable(GL_CULL_FACE);

    SDL_GetWindowSize(m_window, &m_width, &m_height);
    m_mainFrameBuffer = new engine::FboInfo(m_width, m_height, 1);
    m_flareManager = new engine::FlareManager(0.16f, m_width, m_height);
    m_postfxManager = new engine::PostFxManager(m_width, m_height);
}

bool Game::update()
{
    // TODO here we will update the game state (model transfroms, effects etc)
    bool exitGame = handleEvents();

    return exitGame;
}

bool Game::handleEvents()
{
    // check events (keyboard among other)
    SDL_Event event;
    bool quitEvent = false;
    engine::Camera* camera = m_scene->getCamera();
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
            quitEvent = true;
        }
        if (event.type == SDL_MOUSEMOTION && !ImGui::IsMouseHoveringAnyWindow()) {
            // More info at https://wiki.libsdl.org/SDL_MouseMotionEvent
            static int prev_xcoord = event.motion.x;
            static int prev_ycoord = event.motion.y;
            int delta_x = event.motion.x - prev_xcoord;
            int delta_y = event.motion.y - prev_ycoord;

            if (event.button.button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                float rotationSpeed = 0.0005f;
                glm::mat4 yaw = glm::rotate(rotationSpeed * -delta_x, camera->getWorldUp());
                glm::mat4 pitch = glm::rotate(rotationSpeed * -delta_y, glm::normalize(glm::cross(camera->getDirection(), camera->getWorldUp())));
                camera->setDirection(glm::vec3(pitch * yaw * glm::vec4(camera->getDirection(), 0.0f)));
            }

            prev_xcoord = event.motion.x;
            prev_ycoord = event.motion.y;
        }
    }

    // check keyboard state (which keys are still pressed)
    const uint8_t *state = SDL_GetKeyboardState(nullptr);
    glm::vec3 cameraRight = glm::cross(camera->getDirection(), camera->getWorldUp());

    if (state[SDL_SCANCODE_W]) {
        camera->setPosition(camera->getPosition() + (camera->getCameraSpeed() * camera->getDirection()));
    }
    if (state[SDL_SCANCODE_S]) {
        camera->setPosition(camera->getPosition() - (camera->getCameraSpeed() * camera->getDirection()));
    }
    if (state[SDL_SCANCODE_A]) {
        camera->setPosition(camera->getPosition() - (camera->getCameraSpeed() * cameraRight));
    }
    if (state[SDL_SCANCODE_D]) {
        camera->setPosition(camera->getPosition() + (camera->getCameraSpeed() * cameraRight));
    }
    if (state[SDL_SCANCODE_Q]) {
        camera->setPosition(camera->getPosition() - (camera->getCameraSpeed() * camera->getWorldUp()));
    }
    if (state[SDL_SCANCODE_E]) {
        camera->setPosition(camera->getPosition() + (camera->getCameraSpeed() * camera->getWorldUp()));
    }

    return quitEvent;
}

void Game::render()
{
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), float(m_width) / float(m_height), 3.0f, 1000.0f);
    glm::mat4 viewMatrix = glm::lookAt(
        m_scene->getCamera()->getPosition(), 
        m_scene->getCamera()->getPosition() + m_scene->getCamera()->getDirection(), 
        m_scene->getCamera()->getWorldUp());

    // render environment
    glBindFramebuffer(GL_FRAMEBUFFER, m_mainFrameBuffer->getFrameBufferId());
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.2, 0.2, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_environmentManager->getEnvironmentProgram());
    m_postfxManager->setShaderValues(m_currentEffect, m_environmentManager->getEnvironmentProgram());
    m_environmentManager->renderEnvironment(projectionMatrix, viewMatrix, m_scene->getCamera()->getPosition());
    
    // render terrain
    m_postfxManager->setShaderValues(m_currentEffect, m_heightfield.useProgram());
    m_heightfield.render(viewMatrix, projectionMatrix, m_scene->getCamera()->getPosition(), m_environmentManager->getEnvironmentMultiplier(), m_scene->getSun());

    // render scene
    glUseProgram(m_scene->getSceneProgram());
    m_postfxManager->setShaderValues(m_currentEffect, m_scene->getSceneProgram());
    m_scene->render(projectionMatrix, viewMatrix, m_environmentManager->getEnvironmentMultiplier());

    // render post fx
    m_postfxManager->renderPostFx(m_currentEffect, m_mainFrameBuffer, projectionMatrix, viewMatrix);

    if (m_useLensFlare)
    {
        // TODO pass suns position
        m_flareManager->render(viewMatrix, projectionMatrix, m_scene->getSunPosition());
    }

    CHECK_GL_ERROR();

    gui();
    SDL_GL_SwapWindow(m_window);
}

void Game::gui()
{
    // Inform imgui of new frame
    ImGui_ImplSdlGL3_NewFrame(m_window);

    using namespace engine;

    // ----------------- Set variables --------------------------
    ImGui::Text("Post-processing effect");
    ImGui::RadioButton("None", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::None));
    ImGui::RadioButton("Sepia", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Sepia));
    ImGui::RadioButton("Mushroom", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Mushroom));
    ImGui::RadioButton("Blur", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Blur));
    ImGui::SameLine();
    //ImGui::SliderInt("Filter size", &filterSize, 1, 12); // TODO filter size for bloom
    ImGui::RadioButton("Grayscale", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Grayscale));
    ImGui::RadioButton("All of the above", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Composition));
    ImGui::RadioButton("Mosaic", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Mosaic));
    ImGui::RadioButton("Separable Blur", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Separable_blur));
    ImGui::RadioButton("Bloom", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Bloom));
    ImGui::RadioButton("Motion blur", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Motion_Blur));
    ImGui::RadioButton("DOF", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::DOF));
    ImGui::RadioButton("Pseudo Lens flare", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Pseudo_Lens_Flare));
    ImGui::Text("Lens flare");
    ImGui::RadioButton("Lens flare on", (int*)&m_useLensFlare, true);
    ImGui::RadioButton("Lens flare off", (int*)&m_useLensFlare, false);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    // ----------------------------------------------------------

    // Render the GUI.
    ImGui::Render();
}

void Game::destroy()
{
    m_modelManager->destroy();
    m_flareManager->destroy();
    m_environmentManager->destroy();
    m_postfxManager->destroy();
    m_mainFrameBuffer->destroy();
    m_scene->destroy();
    delete m_modelManager;
    delete m_flareManager;
    delete m_environmentManager;
    delete m_postfxManager;
    delete m_mainFrameBuffer;
    delete m_scene;
    engine::shutDown(m_window);
}