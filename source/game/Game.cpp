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

#define TEXT_X_POSITION 920

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

    //setup scene
    m_scene = new engine::Scene(glm::vec3(112.0f, 15.0f, 82.0f), glm::normalize(glm::vec3(0.0f) - glm::vec3(0.14f, -0.047f, 0.989f)));
    // Load some models.
    glm::mat4 modelMatrix = glm::translate(glm::vec3(110.0f, 100.0f, 70.0f));
    m_modelManager->createModel("../scenes/helicopter.obj", static_cast<unsigned int>(GameModels::HelicopModel), modelMatrix);

    m_modelManager->createModel("../scenes/helicopterBlades.obj", static_cast<unsigned int>(GameModels::HelicopterBladesModel), modelMatrix);

    
    modelMatrix = glm::translate(glm::vec3(-70.0f, 15.0f, 70.0f));
    m_modelManager->createModel("../scenes/photocameraRotated.obj", static_cast<unsigned int>(GameModels::CameraModel), modelMatrix);
    
    CameraBoundingBox* cameraBoundingBox = new CameraBoundingBox(
        glm::vec3(5.0f, 10.0f, 5.0f), 0.0f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::CameraModel)), m_scene->getCamera());
    m_collisionManager->addDynamicCollider(cameraBoundingBox);

    // tree dimensions x = 10 y = 20 z = 10

    std::vector<engine::Model*> treeModels;

    modelMatrix = glm::translate(glm::vec3(65.0, 10.0, 65.0));
    treeModels.push_back(m_modelManager->createModel("../scenes/treeDecimated.obj", static_cast<unsigned int>(GameModels::TreeModel), modelMatrix));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix); 

    modelMatrix = glm::translate(glm::vec3(35.0, 10.0, 10.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
       
    modelMatrix = glm::translate(glm::vec3(30.0, 10.0, 5.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
        
    modelMatrix = glm::translate(glm::vec3(30.0, 10.0, 15.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
         
    modelMatrix = glm::translate(glm::vec3(30.0, 10.0, 20.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
         
    modelMatrix = glm::translate(glm::vec3(35.0, 10.0, 25.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(70.0, 10.0, 65.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(70.0, 10.0, 70.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);

    modelMatrix = glm::translate(glm::vec3(35.0, 10.0, 30.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(40.0, 10.0, 30.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);

    modelMatrix = glm::translate(glm::vec3(40.0, 10.0, 35.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(45.0, 10.0, 35.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(45.0, 10.0, 40.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(50.0, 10.0, 40.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(40.0, 10.0, 40.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(40.0, 10.0, -10.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(30.0, 10.0, -10.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(35.0, 10.0, 0.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(30.0, 10.0, -5.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(20.0, 10.0, -5.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(110.0, 10.0, 110.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(100.0, 10.0, 120.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(100.0, 10.0, 110.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(90.0, 10.0, 110.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(90.0, 10.0, 120.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(90.0, 10.0, 110.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);

    modelMatrix = glm::translate(glm::vec3(95.0, 10.0, 115.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);

    modelMatrix = glm::translate(glm::vec3(135.0, 10.0, 30.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);
    
    modelMatrix = glm::translate(glm::vec3(145.0, 10.0, 30.0));
    treeModels.push_back(m_modelManager->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), static_cast<unsigned int>(GameModels::TreeModel)));
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::TreeModel)), modelMatrix);

    for (auto model : treeModels)
    {
        engine::BoundingBox* treeBoundingBox = new engine::BoundingBox(glm::vec3(1.0f, 20.0f, 1.0f), 0.0f,model);
        m_collisionManager->addStaticCollider(treeBoundingBox);
    }

    modelMatrix = glm::translate(glm::vec3(100.0, 17.8, 30.0));
    m_modelManager->createModel("../scenes/house.obj", static_cast<unsigned int>(GameModels::HouseModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::HouseModel)), modelMatrix);
    engine::BoundingBox* houseBoundingBox = new engine::BoundingBox(glm::vec3(27.0f, 10.0f, 15.0f), -0.227f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::HouseModel)));
    m_collisionManager->addStaticCollider(houseBoundingBox);

    modelMatrix = glm::translate(glm::vec3(80.0, 13.0, 70.0));
    m_modelManager->createModel("../scenes/well.obj", static_cast<unsigned int>(GameModels::WellModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::WellModel)), modelMatrix);
    engine::BoundingBox* wellBoundingBox = new engine::BoundingBox(glm::vec3(3.2f, 10.0f, 3.2f), 0.0f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::WellModel)));
    m_collisionManager->addStaticCollider(wellBoundingBox);

    modelMatrix = glm::translate(glm::vec3(150.0, 11.4, 60.0));
    m_modelManager->createModel("../scenes/bench.obj", static_cast<unsigned int>(GameModels::BenchModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)), modelMatrix);
    engine::BoundingBox* benchBoundingBox = new engine::BoundingBox(glm::vec3(80.5f, 10.0f, 2.0f), 4.71f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)));
    m_collisionManager->addStaticCollider(benchBoundingBox);

    modelMatrix = glm::translate(glm::vec3(145.0, 11.4, 90.0));
    m_modelManager->createModel("../scenes/bench2.obj", static_cast<unsigned int>(GameModels::BenchModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)), modelMatrix);
    engine::BoundingBox* bench2BoundingBox = new engine::BoundingBox(glm::vec3(80.5f, 10.0f, 2.0f), 3.83f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)));
    m_collisionManager->addStaticCollider(bench2BoundingBox);

    modelMatrix = glm::translate(glm::vec3(60.0, 11.4, 0.0));
    m_modelManager->createModel("../scenes/bench3.obj", static_cast<unsigned int>(GameModels::BenchModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)), modelMatrix);
    engine::BoundingBox* bench3BoundingBox = new engine::BoundingBox(glm::vec3(20.0f, 10.0f, 2.0f), 5.37f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)));
    m_collisionManager->addStaticCollider(bench3BoundingBox);

    modelMatrix = glm::translate(glm::vec3(45.0, 11.4, 30.0));
    m_modelManager->createModel("../scenes/bench4.obj", static_cast<unsigned int>(GameModels::BenchModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)), modelMatrix);
    engine::BoundingBox* bench4BoundingBox = new engine::BoundingBox(glm::vec3(40.0f, 10.0f, 2.0f), 2.04f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)));
    m_collisionManager->addStaticCollider(bench4BoundingBox);
    
    modelMatrix = glm::translate(glm::vec3(80.0, 11.4, 90.0));
    m_modelManager->createModel("../scenes/bench4.obj", static_cast<unsigned int>(GameModels::BenchModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)), modelMatrix);
    engine::BoundingBox* bench4BoundingBox2 = new engine::BoundingBox(glm::vec3(15.0f, 10.0f, 2.0f), 2.04f, m_modelManager->getModel(static_cast<unsigned int>(GameModels::BenchModel)));
    m_collisionManager->addStaticCollider(bench4BoundingBox2);

    modelMatrix = glm::translate(glm::vec3(140.0, 9.8, 90.0));
    m_modelManager->createModel("../scenes/VictoryGuy.obj", static_cast<unsigned int>(GameModels::VictoryGuyModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::VictoryGuyModel)), modelMatrix);

    modelMatrix = glm::translate(glm::vec3(110.0f, 11.2f, 70.0f));
    m_modelManager->createModel("../scenes/helipad.obj", static_cast<unsigned int>(GameModels::HeliPadModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::HeliPadModel)), modelMatrix);

    /*modelMatrix = glm::translate(glm::vec3(110.0f,15.0f, 70.0f));
    m_modelManager->createModel("../scenes/testDice.obj", static_cast<unsigned int>(GameModels::HeliPadModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::HeliPadModel)), modelMatrix);*/

    modelMatrix = glm::translate(glm::vec3(110.0f, 15.0f, 70.0f));
    m_modelManager->createModel("../scenes/startScreen.obj", static_cast<unsigned int>(GameModels::StartScreenModel), modelMatrix);
    m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::StartScreenModel)), modelMatrix);



    //setup terrain
    m_heightfield.generateMesh(512);
    m_heightfield.loadHeightField("../scenes/nlsFinland/terrainHeightmap.png");
    m_heightfield.loadDiffuseTexture("../scenes/nlsFinland/testText.jpg");

    // setup quests
    m_questManager.setFirstQuestComplete(false);
    m_questManager.addQuest("Take a photo of this amazing helipad.", glm::vec3(120.5f, 15.0f, 65.7f), glm::vec3(-0.805f, -0.464f, 0.370f), static_cast<int>(engine::PostFxManager::PostFxTypes::None), false);
    m_questManager.addQuest("Take a photo of this beautiful lovely gentleman.", glm::vec3(131.0f, 15.0f, 87.0f), glm::vec3(0.975f, -0.111f, 0.190f), static_cast<int>(engine::PostFxManager::PostFxTypes::Sepia), false);
    m_questManager.addQuest("Take a photo of this beautiful house from this angle.", glm::vec3(90.0f, 15.0f, 49.5f), glm::vec3(0.676f, 0.349f, -0.649f), static_cast<int>(engine::PostFxManager::PostFxTypes::DOF), false);
    m_questManager.addQuest("Take a photo of this beautiful sunset over the house in Greyscale.", glm::vec3(113.5f, 15.0f, 91.5f), glm::vec3(-0.458f, 0.173f, -0.872f), static_cast<int>(engine::PostFxManager::PostFxTypes::Grayscale), true);
    m_questManager.addQuest("Take a photo of this beautiful sunset over the house.", glm::vec3(113.5f, 15.0f, 91.5f), glm::vec3(-0.458f, 0.173f, -0.872f), static_cast<int>(engine::PostFxManager::PostFxTypes::DOF), true);
    m_questManager.addQuest("Take a photo of this lovely view.", glm::vec3(75.0f, 15.0f, 58), glm::vec3(0.729f, 0.001f, 0.685f), static_cast<int>(engine::PostFxManager::PostFxTypes::Bloom), false);
    m_questManager.addQuest("Take a photo of this little bench pixelated.", glm::vec3(55.5f, 15.0f, 24.5f), glm::vec3(-0.873f, -0.110f, 0.46f), static_cast<int>(engine::PostFxManager::PostFxTypes::Mosaic), false);
    m_questManager.addQuest("Take a photo of this little bench.", glm::vec3(55.5f, 15.0f, 24.5f), glm::vec3(-0.873f, -0.110f, 0.46f), static_cast<int>(engine::PostFxManager::PostFxTypes::None), false);
    m_questManager.addQuest("Take a photo of this nice tree.", glm::vec3(102.0f, 15.0f, 88.0f), glm::vec3(0.473f, 0.194f, 0.859f), static_cast<int>(engine::PostFxManager::PostFxTypes::None), false);
    m_questManager.addQuest("Take a photo of this amazing starting logo.", glm::vec3(112.0f, 15.0f, 82.0f), glm::vec3(-0.14f, 0.047f, -0.989f), static_cast<int>(engine::PostFxManager::PostFxTypes::None), false);
    //m_questManager.addQuest("Take a photo of this nice tree.", glm::vec3(-70.0f, 15.0f, 70.0f), glm::normalize(glm::vec3(0.0f) - glm::vec3(-70.0f, 15.0f, 70.0f)), static_cast<int>(engine::PostFxManager::PostFxTypes::None), false);

    Credit credit("Authors: Marek and Orestis", glm::ivec2(TEXT_X_POSITION, 900));
    m_creditsManager.addCredit(credit);
    Credit credit2("THANKS FOR PLAYING!", glm::ivec2(TEXT_X_POSITION, 700));
    m_creditsManager.addCredit(credit2);

    // setup game variables
    m_currentEffect = engine::PostFxManager::PostFxTypes::None;
    m_currentScreenEffect = engine::PostFxManager::PostFxTypes::None;
    m_useLensFlare = false;
    m_gameCameraActive = false;
    m_showColliders = false;
    m_showTeselatedTerrain = false;
    m_gameFinished = false;
    m_creditsStarted = false;
    m_flashTime = 0.0f;
    m_fadeOutTime = 0.0f;

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
    bool exitGame = false;
    if (m_creditsStarted)
    {
        if (m_fadeOutTime <= 0.0f)
        {
            m_creditsManager.start(m_window);
            m_creditsManager.updateCredits();
            m_creditsManager.renderCredits();
            exitGame = m_creditsManager.creditsFinished();
            if (!exitGame)
            {
                exitGame = handleEvents();
            }
        }
    }
    else
    {
        exitGame = handleEvents();
        m_collisionManager->checkCollision();
        if ((m_questManager.setQuestComplete() == 0) && (!m_gameFinished))
        {
            m_gameFinished = true;
            m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::HelicopModel)));
            m_scene->addModel(m_modelManager->getModel(static_cast<unsigned int>(GameModels::HelicopterBladesModel)));
        }

        if (m_gameFinished)
        {
            engine::Model* heliModel = m_modelManager->getModel(static_cast<unsigned int>(GameModels::HelicopModel));
            engine::Model* heliModelBlades = m_modelManager->getModel(static_cast<unsigned int>(GameModels::HelicopterBladesModel));
            if (heliModel->m_modelMatrix[3][1] > 15.0f)
            {
                heliModelBlades->m_modelMatrix = glm::rotate(heliModelBlades->m_modelMatrix, 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                heliModel->m_modelMatrix[3][1] -= 0.5f;
                heliModelBlades->m_modelMatrix[3][1] -= 0.5f;
            }
            else if ((m_scene->getCamera()->getPosition().x < 115.0f) && (m_scene->getCamera()->getPosition().x > 105.0f) &&
                (m_scene->getCamera()->getPosition().z < 75.0f) && (m_scene->getCamera()->getPosition().z > 65.0f))
            {               
                m_creditsStarted = true;
                m_fadeOutTime = 1.0f;
            }
            else 
            {
                heliModelBlades->m_modelMatrix = glm::rotate(heliModelBlades->m_modelMatrix, 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }
    }
    
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
    while (SDL_PollEvent(&event)) 
    {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) 
        {
            quitEvent = true;
        }
        if (event.type == SDL_MOUSEMOTION && !ImGui::IsMouseHoveringAnyWindow() && m_questManager.getFirstQuestComplete()) 
        {
            // More info at https://wiki.libsdl.org/SDL_MouseMotionEvent
            static int prev_xcoord = event.motion.x;
            static int prev_ycoord = event.motion.y;
            int delta_x = event.motion.x - prev_xcoord;
            int delta_y = event.motion.y - prev_ycoord;

            if (event.button.button & SDL_BUTTON(SDL_BUTTON_LEFT)) 
            {
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
    if (m_questManager.getFirstQuestComplete() && !m_creditsStarted)
    {
        if (state[SDL_SCANCODE_W])
        {
            cameraPosition += camera->getCameraSpeed() * glm::vec3(camera->getDirection().x, 0.0f, camera->getDirection().z);
        }
        if (state[SDL_SCANCODE_S])
        {
            cameraPosition -= camera->getCameraSpeed() * glm::vec3(camera->getDirection().x, 0.0f, camera->getDirection().z);
        }
        if (state[SDL_SCANCODE_A])
        {
            cameraPosition -= camera->getCameraSpeed() * cameraRight;
        }
        if (state[SDL_SCANCODE_D])
        {
            cameraPosition += camera->getCameraSpeed() * cameraRight;
        }
        m_modelManager->disableModel(static_cast<unsigned int>(GameModels::StartScreenModel));
    }
    if (state[SDL_SCANCODE_E])
    {
        m_gameCameraActive = true;
    }
    if (state[SDL_SCANCODE_Q])
    {
        m_gameCameraActive = false;
    }
    if (state[SDL_SCANCODE_F] && m_gameCameraActive &&
        m_questManager.checkQuestCompletion(camera->getPosition(), camera->getDirection(), static_cast<int>(m_currentEffect), m_useLensFlare))
    {
        m_flashTime = 1.0f;
    }
    camera->setPosition(cameraPosition);
    glm::mat4 modelMatrix = glm::translate(camera->getPosition() + (camera->getDirection() * 5.0f));
    m_gameCamera->updateCameraMatrix(modelMatrix);

    return quitEvent;
}

void Game::render()
{
    if (!(m_creditsStarted && m_fadeOutTime <= 0.0f))
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
        m_scene->renderScene(projectionMatrix, viewMatrix, m_environmentManager->getEnvironmentMultiplier(), m_showNormalMap);

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
        if (m_flashTime > 0.0f)
        {
            m_gameCamera->renderCameraFlash(m_flashTime, glm::vec3(1.0f));
            m_flashTime -= 0.01f;
        }
        if (m_fadeOutTime > 0.0f)
        {
            m_gameCamera->renderCameraFlash(1.0f - m_fadeOutTime, glm::vec3(0.0f));
            m_fadeOutTime -= 0.01f;
        }

        CHECK_GL_ERROR();

        gui();
        SDL_GL_SwapWindow(m_window);
    }
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
        ImGui::SameLine();
        //ImGui::SliderInt("Filter size", &filterSize, 1, 12); // TODO filter size for bloom
        ImGui::RadioButton("Grayscale", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Grayscale));
        ImGui::RadioButton("Mosaic", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Mosaic));
        ImGui::RadioButton("Separable Blur", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Separable_blur));
        ImGui::RadioButton("Bloom", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Bloom));
        ImGui::RadioButton("Motion blur", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::Motion_Blur));
        ImGui::RadioButton("DOF", (int*)&m_currentEffect, static_cast<int>(PostFxManager::PostFxTypes::DOF));
        // lens flare button
        if (m_useLensFlare == true)
        {
            ImGui::PushID(" Show lens flare ");
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 1));
            ImGui::Button(" Show lens flare ");
            if (ImGui::IsItemClicked(0))
            {
                m_useLensFlare = !m_useLensFlare;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
        else
        {
            if (ImGui::Button(" Show lens flare "))
            {
                m_useLensFlare = true;
            }
        }
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
    // show colliders button
    if (m_showColliders == true)
    {
        ImGui::PushID(" Show colliders ");
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 1));
        ImGui::Button(" Show colliders ");
        if (ImGui::IsItemClicked(0))
        {
            m_showColliders = !m_showColliders;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    else
    {
        if (ImGui::Button(" Show colliders "))
        {
            m_showColliders = true;
        }
    }
    // terrain tesselation button
    if (m_showTeselatedTerrain == true)
    {
        ImGui::PushID(" Show terrain tesselation ");
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 1));
        ImGui::Button(" Show terrain tesselation ");
        if (ImGui::IsItemClicked(0))
        {
            m_showTeselatedTerrain = !m_showTeselatedTerrain;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    else
    {
        if (ImGui::Button(" Show terrain tesselation "))
        {
            m_showTeselatedTerrain = true;
        }
    }
    // normal maps button
    if (m_showNormalMap == true)
    {
        ImGui::PushID(" Show with Normal Maps ");
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 1));
        ImGui::Button(" Show with Normal Maps ");
        if (ImGui::IsItemClicked(0))
        {
            m_showNormalMap = !m_showNormalMap;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    else
    {
        if (ImGui::Button(" Show with Normal Maps "))
        {
            m_showNormalMap = true;
        }
    }
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // Render the GUI.
    ImGui::Render();
}

void Game::destroy()
{
    m_collisionManager->destroy();
    m_heightfield.destroy();
    m_modelManager->destroy();
    m_flareManager->destroy();
    m_environmentManager->destroy();
    m_postfxManager->destroy();
    m_mainFrameBuffer->destroy();
    m_scene->destroy();
    delete m_collisionManager;
    delete m_modelManager;
    delete m_flareManager;
    delete m_environmentManager;
    delete m_postfxManager;
    delete m_mainFrameBuffer;
    delete m_scene;
    engine::shutDown(m_window);
}