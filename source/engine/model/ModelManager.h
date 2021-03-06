#pragma once
#include <unordered_map>
#include <glm/glm.hpp>

namespace engine
{
    class Model;
    class ModelManager
    {
    public:
        Model* createModel(const char* path, unsigned int modelId, const glm::mat4& modelMatrix);
        Model* getModel(unsigned int modelId);
        Model* addModel(Model* model, unsigned int modelId);
        void disableModel(unsigned int modelId);
        void destroy();

    private:
        std::unordered_map<unsigned int, Model*> m_models;
    };
}