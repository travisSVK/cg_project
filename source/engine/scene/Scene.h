#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

namespace engine
{
    class Camera;
    class Model;
    class Sun;
    class Scene
    {
    public:

        Scene(const glm::vec3& cameraPosition, const glm::vec3& cameraDirection);
        void render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, float environmentMultiplier);
        void addModel(Model* model);
        Camera* getCamera();
        glm::vec3 getSunPosition();
        GLuint getSceneProgram();
        Sun* getSun();
        void destroy();

    private:
        Camera* m_camera;
        Sun* m_sun;
        std::vector<Model*> m_models;
        GLuint m_sceneProgram;
    };
}