#include "Scene.h"
#include "../model/Model.h"
#include "Camera.h"
#include "../Helper.h"

namespace engine
{
    Scene::Scene(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection)
    {
        m_camera = new Camera(cameraPosition, cameraDirection);
        m_sceneProgram = engine::loadShaderProgram("shaders/simple.vert", "shaders/simple.frag", "", "");
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
        // vec4 viewSpaceLightPosition = view * vec4(lightPosition, 1.0f); // TODO this will be sun position
        // engine::setUniformSlow(shaderProgram, "point_light_color", point_light_color); // TODO this will be sun color
        // engine::setUniformSlow(shaderProgram, "point_light_intensity_multiplier", point_light_intensity_multiplier); // TODO this will be sun intensity ultiplier
        // engine::setUniformSlow(shaderProgram, "viewSpaceLightPosition", vec3(viewSpaceLightPosition)); // TODO this will be sun position

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
    }

    void Scene::destroy()
    {
        glDeleteProgram(m_sceneProgram);
        delete m_camera;
    }
}