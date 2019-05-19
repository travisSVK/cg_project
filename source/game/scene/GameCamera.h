#pragma once
#include "glm/glm.hpp"
#include "GL/glew.h"

namespace engine 
{
    class Model;
    class FboInfo;
}

class GameCamera
{
public:

    GameCamera(engine::Model* gameCameraModel, int height, int width);
    void updateCameraMatrix(const glm::mat4& gameCameraMatrix);
    void activateGameCameraFB();
    void renderGameCamera(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    engine::FboInfo* getGameCameraFB();
    void setParametersForCameraDisplay(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void resetParametersForCameraDisplay(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void renderCameraFlash(float flashTime);

private:
    engine::Model* m_cameraModel;
    engine::FboInfo* m_cameraFrameBuffer;
    GLuint m_cameraFlashProgram;
};