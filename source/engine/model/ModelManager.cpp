#include "ModelManager.h"
#include "Model.h"

namespace engine
{
    void ModelManager::createModel(const char* path, unsigned int modelId, const glm::mat4& modelMatrix)
    {
        Model* model = engine::loadModelFromOBJ(path);
        model->m_modelMatrix = modelMatrix;
        m_models[modelId] = model;
    }

    Model* ModelManager::getModel(unsigned int modelId)
    {
        return m_models[modelId];
    }

    void ModelManager::destroy()
    {
        for (auto it : m_models)
        {
            freeModel(it.second);
        }
    }
}