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
#include "collision/CollisionManager.h"
#include "collision/SATCollisionStrategy.h"
#include "collision/BoundingBox.h"
#include "FboInfo.h"
#include "scene/Scene.h"
#include "scene/Sun.h"
#include "scene/Camera.h"
#include "Helper.h"
#include "model/Model.h"
// game spacific includes
#include "scene/GameCamera.h"
#include "collision/CameraBoundingBox.h"
#include "Enums.h"


void Game::initialize()
{
    m_window = engine::init_window_SDL("Game", 1920, 1080);

    m_modelManager = new engine::ModelManager();
    m_environmentManager = new engine::EnvironmentManager(1.5f);
    m_collisionManager = new engine::CollisionManager(new engine::SATCollisionStrategy());
    
    
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

    //setup scene
    m_scene = new engine::Scene(glm::vec3(-70.0f, 15.0f, 70.0f), glm::normalize(glm::vec3(0.0f) - glm::vec3(-70.0f, 15.0f, 70.0f)));
    // Load some models.
    glm::mat4 modelMatrix = glm::translate(60.0f * m_scene->getCamera()->getWorldUp());
    //m_modelManager->createModel("../scenes/NewShip.obj", static_cast<unsigned int>(GameModels::FighterModel), modelMatrix);
    //m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::FighterModel)));
    modelMatrix = glm::translate(glm::vec3(-70.0f, 15.0f, 70.0f));
    m_modelManager->createModel("../scenes/photocameraRotated.obj", static_cast<unsigned int>(GameModels::CameraModel), modelMatrix);
    
    CameraBoundingBox* cameraBoundingBox = new CameraBoundingBox(
        glm::vec3(5.0f, 10.0f, 5.0f), 0.0f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::CameraModel)), m_scene->getCamera());
    m_collisionManager->addDynamicCollider(cameraBoundingBox);

    // tree dimensions x = 10 y = 20 z = 10
    modelMatrix = glm::translate(glm::vec3(-60.0f, 10.0f, 60.0f));
    m_modelManager->createModel("../scenes/treeDecimated.obj", static_cast<unsigned int>(GameModels::TreeModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    engine::BoundingBox* treeBoundingBox = new engine::BoundingBox(glm::vec3(1.0f, 20.0f, 1.0f), 0.0f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)));
    m_collisionManager->addStaticCollider(treeBoundingBox);
    
    /*modelMatrix = glm::translate(glm::vec3(65.0, 10.0, 65.0));
    m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(70.0, 10.0, 65.0));
    m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(70.0, 10.0, 70.0));
    m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(110.0, 10.0, 110.0));
    m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(100.0, 10.0, 120.0));
    m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(100.0, 10.0, 110.0));
    m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(90.0, 10.0, 110.0));
    m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(90.0, 10.0, 120.0));
    m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(90.0, 10.0, 110.0));
    m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);*/

    modelMatrix = glm::translate(glm::vec3(100.0, 17.8, 30.0));
    m_modelManager->createModel("../scenes/house.obj", static_cast<unsigned int>(GameModels::HouseModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::HouseModel)), modelMatrix);
    engine::BoundingBox* houseBoundingBox = new engine::BoundingBox(glm::vec3(27.0f, 10.0f, 15.0f), -0.227f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::HouseModel)));
    m_collisionManager->addStaticCollider(houseBoundingBox);

    modelMatrix = glm::translate(glm::vec3(80.0, 13.0, 70.0));
    m_modelManager->createModel("../scenes/well.obj", static_cast<unsigned int>(GameModels::WellModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::WellModel)), modelMatrix);
     
    //setup terrain
    m_heightfield.generateMesh(64);
    m_heightfield.loadHeightField("../scenes/nlsFinland/terrainHeightmap.png");
    m_heightfield.loadDiffuseTexture("../scenes/nlsFinland/testText.jpg");

    // setup quests
    m_questManager.addQuest("Take a photo of this beautiful sunset over house.", glm::vec3(137.5f, 15.0f, 109.0f), glm::vec3(-0.437f, 0.06f, -0.897f), static_cast<int>(engine::PostFxManager::PostFxTypes::None), true);
    m_questManager.addQuest("Take a photo of this beautiful house.", glm::vec3(74.0f, 15.0f, 76.0f), glm::vec3(0.388f, 0.002f, -0.922f), static_cast<int>(engine::PostFxManager::PostFxTypes::DOF), false);
    m_questManager.addQuest("Take a photo of this nice tree.", glm::vec3(-70.0f, 15.0f, 70.0f), glm::normalize(glm::vec3(0.0f) - glm::vec3(-70.0f, 15.0f, 70.0f)), static_cast<int>(engine::PostFxManager::PostFxTypes::None), false);

    // setup game variables
    m_currentEffect = engine::PostFxManager::PostFxTypes::None;
    m_currentScreenEffect = engine::PostFxManager::PostFxTypes::None;
    m_useLensFlare = false;
    m_gameCameraActive = false;
    m_showColliders = false;
    m_showTeselatedTerrain = false;

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
    m_gameCamera = new GameCamera(m_modelManager->getModel(static_cast<unsigned int>(GameModels::CameraModel)), m_width, m_height);
}

bool Game::update()
{
    bool exitGame = handleEvents();
    m_collisionManager->checkCollision();
    m_questManager.setQuestComplete();

    return exitGame;
}

bool Game::handleEvents()
{
    // check events (keyboard among other)
    SDL_Event event;
    bool quitEvent = false;
    engine::Camera* camera = m_scene->getCamera();
    glm::mat4 gameCameraYaw = glm::rotate(0.0f, camera->getWorldUp());;
    glm::mat4 gameCameraPitch = glm::rotate(0.0f, glm::normalize(glm::cross(camera->getDirection(), camera->getWorldUp())));
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
                gameCameraYaw = glm::rotate(0.0f, camera->getWorldUp());
                gameCameraPitch = glm::rotate(0.0f, glm::normalize(glm::cross(camera->getDirection(), camera->getWorldUp())));
                camera->setDirection(glm::vec3(pitch * yaw * glm::vec4(camera->getDirection(), 0.0f)));
            }

            prev_xcoord = event.motion.x;
            prev_ycoord = event.motion.y;
        }
    }

    // check keyboard state (which keys are still pressed)
    const uint8_t *state = SDL_GetKeyboardState(nullptr);
    glm::vec3 cameraRight = glm::cross(camera->getDirection(), camera->getWorldUp());
    glm::vec3 cameraPosition = camera->getPosition();
    if (state[SDL_SCANCODE_W]) {
        cameraPosition += camera->getCameraSpeed() * glm::vec3(camera->getDirection().x, 0.0f, camera->getDirection().z);
        //camera->setPosition(camera->getPosition() + (camera->getCameraSpeed() * glm::vec3(camera->getDirection().x, 0.0f, camera->getDirection().z)));
    }
    if (state[SDL_SCANCODE_S]) {
        cameraPosition -= camera->getCameraSpeed() * glm::vec3(camera->getDirection().x, 0.0f, camera->getDirection().z);
        //camera->setPosition(camera->getPosition() - (camera->getCameraSpeed() * glm::vec3(camera->getDirection().x, 0.0f, camera->getDirection().z)));
    }
    if (state[SDL_SCANCODE_A]) {
        cameraPosition -= camera->getCameraSpeed() * cameraRight;
        //camera->setPosition(camera->getPosition() - (camera->getCameraSpeed() * cameraRight));
    }
    if (state[SDL_SCANCODE_D]) {
        cameraPosition += camera->getCameraSpeed() * cameraRight;
        //camera->setPosition(camera->getPosition() + (camera->getCameraSpeed() * cameraRight));
    }
    if (state[SDL_SCANCODE_E]) {
        m_gameCameraActive = true;
    }
    if (state[SDL_SCANCODE_Q]) {
        m_gameCameraActive = false;
    }
    if (state[SDL_SCANCODE_F] && m_gameCameraActive) {
        m_questManager.checkQuestCompletion(camera->getPosition(), camera->getDirection(), static_cast<int>(m_currentEffect), m_useLensFlare);
    }
    camera->setPosition(cameraPosition);
    glm::mat4 modelMatrix = glm::translate(camera->getPosition() + (camera->getDirection() * 5.0f));
    m_gameCamera->updateCameraMatrix(modelMatrix);

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
    
    //render sun
    /*glUseProgram(m_scene->getSceneProgram());
    m_postfxManager->setShaderValues(m_currentEffect, m_scene->getSceneProgram());
    m_scene->renderSun(projectionMatrix, viewMatrix);*/

    // render terrain
    m_postfxManager->setShaderValues(m_currentEffect, m_heightfield.useProgram());
    m_heightfield.useProgram();
    m_heightfield.render(viewMatrix, projectionMatrix, m_scene->getCamera()->getPosition(), m_environmentManager->getEnvironmentMultiplier(), m_scene->getSun(), m_showTeselatedTerrain);

    // render scene
    glUseProgram(m_scene->getSceneProgram());
    m_postfxManager->setShaderValues(m_currentEffect, m_scene->getSceneProgram());
    m_scene->renderScene(projectionMatrix, viewMatrix, m_environmentManager->getEnvironmentMultiplier());
    
    if (m_gameCameraActive)
    {
        m_postfxManager->renderPostFx(m_currentEffect, m_mainFrameBuffer, m_gameCamera->getGameCameraFB(), projectionMatrix, viewMatrix);
        if (m_useLensFlare)
        {
            m_flareManager->render(viewMatrix, projectionMatrix, m_scene->getSun()->getPosition(), m_mainFrameBuffer->getDepthBuffer());
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_mainFrameBuffer->getFrameBufferId());
        glUseProgram(m_scene->getSceneProgram());
        m_gameCamera->renderGameCamera(m_scene->getSceneProgram(), viewMatrix, projectionMatrix);
    }
    if (m_showColliders)
    {
        m_collisionManager->renderColliders(viewMatrix, projectionMatrix);
    }
    m_postfxManager->renderPostFxToMain(m_currentScreenEffect, m_mainFrameBuffer, projectionMatrix, viewMatrix);
    
    /*if (m_useLensFlare)
    {
        m_flareManager->render(viewMatrix, projectionMatrix, m_scene->getSun()->getPosition());
    }*/
    CHECK_GL_ERROR();

    gui();
    SDL_GL_SwapWindow(m_window);
}

void Game::gui()
{
    // Inform imgui of new frame
    ImGui_ImplSdlGL3_NewFrame(m_window);

    using namespace engine;
    if (m_gameCameraActive)
    {
        ImGui::Begin("Effects");
        ImGui::SetWindowPos(ImVec2(1300, 320));
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
        // ----------------------------------------------------------
        ImGui::End();
    }
    
    ImGui::Begin("Objectives");
    ImGui::SetWindowPos(ImVec2(20, 20));
    ImGui::SetWindowSize(ImVec2(450, (m_questManager.getFinishedQuests().size() * 20) + 135));
    // ----------------- Set variables --------------------------
    for (const auto& quest : m_questManager.getFinishedQuests())
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), quest.m_description.c_str());
        //ImGui::Text(quest.m_description.c_str());
    }
    ImGui::Text(m_questManager.getCurrentQuestObjective().c_str());
    ImGui::Separator();
    ImVec4 positionColor(1.0f, 0.0f, 0.0f, 1);
    ImVec4 directionColor(1.0f, 0.0f, 0.0f, 1);
    if (m_questManager.isCurrentPositionWithinRange(m_scene->getCamera()->getPosition()))
    {
        positionColor = ImVec4(0.0f, 1.0f, 0.0f, 1);
    }
    ImGui::TextColored(positionColor, "Current position: x = %.3f, y = %.3f, z = %.3f", m_scene->getCamera()->getPosition().x, m_scene->getCamera()->getPosition().y, m_scene->getCamera()->getPosition().z);
    if (m_questManager.isCurrentDirectionWithinRange(m_scene->getCamera()->getDirection()))
    {
        directionColor = ImVec4(0.0f, 1.0f, 0.0f, 1);
    }
    ImGui::TextColored(directionColor, "Current direction: x = %.3f, y = %.3f, z = %.3f", m_scene->getCamera()->getDirection().x, m_scene->getCamera()->getDirection().y, m_scene->getCamera()->getDirection().z);
    ImGui::End();

    ImGui::Begin("Debug + screen effects");
    ImGui::SetWindowPos(ImVec2(1500, 20));
    // ----------------- Set variables --------------------------
    ImGui::Text("Post-processing effect");
    ImGui::RadioButton("None", (int*)&m_currentScreenEffect, static_cast<int>(PostFxManager::PostFxTypes::None));
    ImGui::RadioButton("Motion blur", (int*)&m_currentScreenEffect, static_cast<int>(PostFxManager::PostFxTypes::Motion_Blur));
    ImGui::Text("Show colliders");
    ImGui::RadioButton("On", (int*)&m_showColliders, true);
    ImGui::RadioButton("Off", (int*)&m_showColliders, false);
    ImGui::Text("Show terrain tesselation");
    ImGui::RadioButton("Tes On", (int*)&m_showTeselatedTerrain, true);
    ImGui::RadioButton("Tes Off", (int*)&m_showTeselatedTerrain, false);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

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