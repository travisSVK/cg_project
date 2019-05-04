#include "Scene.h"
#include "../model/Model.h"
#include "Camera.h"
#include "../Helper.h"
#include "Sun.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace engine
{
    Scene::Scene(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
    {
        m_camera = new Camera(cameraPosition, cameraDirection);
        m_sceneProgram = engine::loadShaderProgram("shaders/simple.vert", "shaders/simple.frag", "", "");
        m_sun = new Sun(glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
    }

    Camera* Scene::getCamera()
    {
        return m_camera;
    }

    GLuint Scene::getSceneProgram()
    {
        return m_sceneProgram;
    }

    void Scene::addModel(Model* model)
    {
        m_models.push_back(model);
    }

    void Scene::render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, float environmentMultiplier)
    {
        engine::setUniformSlow(m_sceneProgram, "directional_light_color", m_sun->getColor());
        engine::setUniformSlow(m_sceneProgram, "directional_light_intensity_multiplier", m_sun->getIntensityMultiplier());
        glm::mat4 newViewMatrix = viewMatrix;
        newViewMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        engine::setUniformSlow(m_sceneProgram, "viewSpaceLightPosition", glm::vec3(newViewMatrix * glm::vec4(m_sun->getPosition(), 1.0f)));
        engine::setUniformSlow(m_sceneProgram, "environment_multiplier", environmentMultiplier);
        engine::setUniformSlow(m_sceneProgram, "viewInverse", glm::inverse(viewMatrix));
        for (auto model : m_models)
        {
            engine::setUniformSlow(m_sceneProgram, "modelViewProjectionMatrix", projectionMatrix * viewMatrix * model->getModelMatrix());
            engine::setUniformSlow(m_sceneProgram, "modelViewMatrix", viewMatrix * model->getModelMatrix());
            engine::setUniformSlow(m_sceneProgram, "normalMatrix", glm::inverse(glm::transpose(viewMatrix * model->getModelMatrix())));
            engine::setUniformSlow(m_sceneProgram, "modelMatrix", model->getModelMatrix());
            engine::render(model);
        }
        glm::vec3 view = glm::normalize(m_sun->getPosition() - m_camera->getPosition());
        glm::vec3 cameraRight = glm::cross(view, m_camera->getWorldUp());
        glm::vec3 towards = glm::cross(m_camera->getWorldUp(), cameraRight);
        glm::mat4 R(1.0f);
        R[0] = glm::vec4(cameraRight, 0.0f);
        R[1] = glm::vec4(m_camera->getWorldUp(), 0.0f);
        R[2] = glm::vec4(towards, 0.0f);
        m_sun->render(projectionMatrix, viewMatrix, R);
    }

    Sun* Scene::getSun()
    {
        return m_sun;
    }

    glm::vec3 Scene::getSunPosition()
    {
        return m_sun->getPosition();
    }

    void Scene::destroy()
    {
        m_sun->destroy();
        delete m_sun;
        glDeleteProgram(m_sceneProgram);
        delete m_camera;
    }
}