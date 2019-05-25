#include "GameCamera.h"
#include "model/Model.h"
#include "FboInfo.h"
#include "Helper.h"
#include <iostream>
#include <stb_image.h>

GameCamera::GameCamera(engine::Model* gameCameraModel, int width, int height) : m_cameraModel(gameCameraModel)
{
    m_cameraFrameBuffer = new engine::FboInfo(width, height, 1);
    m_cameraFlashProgram = engine::loadShaderProgram("shaders/postFx.vert", "shaders/cameraFlash.frag", "", "");
}

void GameCamera::updateCameraMatrix(const glm::mat4& gameCameraMatrix)
{
    m_cameraModel->m_previousModelMatrix = m_cameraModel->m_modelMatrix;
    m_cameraModel->m_modelMatrix = gameCameraMatrix;
}

void GameCamera::activateGameCameraFB()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_cameraFrameBuffer->getFrameBufferId());
    glViewport(0, 0, m_cameraFrameBuffer->getWidth(), m_cameraFrameBuffer->getHeight());
    glClearColor(0.2, 0.2, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GameCamera::renderGameCamera(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    engine::Material &cameraScreen = m_cameraModel->m_materials[4];
    cameraScreen.m_emission_texture.gl_id = m_cameraFrameBuffer->getColorTextureTarget(0);
    cameraScreen.m_emission_texture.valid = true;
    glm::mat3 inverseRotationMatrix = viewMatrix;
    inverseRotationMatrix = glm::inverse(inverseRotationMatrix);
    glm::mat4 inverseRot = inverseRotationMatrix;
    glm::mat4 scaleMat(1.0f);
    scaleMat[0][0] = 1.5f;
    scaleMat[1][1] = 1.2f;
    m_cameraModel->m_modelMatrix[3][1] -= 0.4f;
    engine::setUniformSlow(program, "modelViewProjectionMatrix", projectionMatrix * viewMatrix * m_cameraModel->getModelMatrix() * inverseRot * scaleMat);
    engine::setUniformSlow(program, "modelViewMatrix", viewMatrix * m_cameraModel->getModelMatrix() * inverseRot);
    engine::setUniformSlow(program, "normalMatrix", glm::inverse(glm::transpose(viewMatrix * m_cameraModel->getModelMatrix() * inverseRot)));
    engine::setUniformSlow(program, "modelMatrix", m_cameraModel->getModelMatrix());
    engine::setUniformSlow(program, "is_camera_model", 1);
    //std::function<void()> set = [this, program, viewMatrix, projectionMatrix]() {setParametersForCameraDisplay(program, viewMatrix, projectionMatrix); };
    //std::function<void()> reset = [this, program, viewMatrix, projectionMatrix]() {resetParametersForCameraDisplay(program, viewMatrix, projectionMatrix); };
    //engine::renderWithDiffOptionForMesh(m_cameraModel, "None", set, reset);
    engine::render(m_cameraModel, false);
}

void GameCamera::setParametersForCameraDisplay(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    engine::setUniformSlow(program, "modelViewProjectionMatrix", projectionMatrix * viewMatrix * m_cameraModel->getModelMatrix());
    engine::setUniformSlow(program, "modelViewMatrix", viewMatrix * m_cameraModel->getModelMatrix());
    engine::setUniformSlow(program, "normalMatrix", glm::inverse(glm::transpose(viewMatrix * m_cameraModel->getModelMatrix())));
    engine::setUniformSlow(program, "modelMatrix", m_cameraModel->getModelMatrix());
}

void GameCamera::resetParametersForCameraDisplay(GLuint program, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    glm::mat3 inverseRotationMatrix = viewMatrix;
    inverseRotationMatrix = glm::inverse(inverseRotationMatrix);
    glm::mat4 inverseRot = inverseRotationMatrix;
    engine::setUniformSlow(program, "modelViewProjectionMatrix", projectionMatrix * viewMatrix * m_cameraModel->getModelMatrix() * inverseRot);
    engine::setUniformSlow(program, "modelViewMatrix", viewMatrix * m_cameraModel->getModelMatrix() * inverseRot);
    engine::setUniformSlow(program, "normalMatrix", glm::inverse(glm::transpose(viewMatrix * m_cameraModel->getModelMatrix() * inverseRot)));
    engine::setUniformSlow(program, "modelMatrix", m_cameraModel->getModelMatrix());
}

void GameCamera::renderCameraFlash(float flashTime, const glm::vec3& color)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(m_cameraFlashProgram);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    engine::setUniformSlow(m_cameraFlashProgram, "flashTime", flashTime);
    engine::setUniformSlow(m_cameraFlashProgram, "color", color);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    engine::drawFullScreenQuad();
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

engine::FboInfo* GameCamera::getGameCameraFB()
{
    return m_cameraFrameBuffer;
}