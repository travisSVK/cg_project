#include "ModelManager.h"
#include "Model.h"

namespace engine
{
    Model* ModelManager::createModel(const char* path, unsigned int modelId, const glm::mat4& modelMatrix)
    {
        Model* model = engine::loadModelFromOBJ(path);
        model->m_enabled = true;
        model->m_modelMatrix = modelMatrix;
        model->m_previousModelMatrix = modelMatrix;
        m_models[modelId] = model;
        return model;
    }

    Model* ModelManager::getModel(unsigned int modelId)
    {
        return m_models[modelId];
    }

    Model* ModelManager::addModel(Model* model, unsigned int modelId)
    {
        Model* model2 = new Model();
        model2->m_enabled = true;
        model2->m_bitTangents = model->m_bitTangents;
        model2->m_bitTangents_bo = model->m_bitTangents_bo;
        model2->m_filename = model->m_filename;
        model2->m_loaded = model->m_loaded;
        model2->m_materials = model->m_materials;
        model2->m_meshes = model->m_meshes;
        model2->m_modelMatrix = model->m_modelMatrix;
        model2->m_name = model->m_name;
        model2->m_normals = model->m_normals;
        model2->m_normals_bo = model->m_normals_bo;
        model2->m_positions = model->m_positions;
        model2->m_positions_bo = model->m_positions_bo;
        model2->m_tangents = model->m_tangents;
        model2->m_tangents_bo = model->m_tangents_bo;
        model2->m_texture_coordinates = model->m_texture_coordinates;
        model2->m_texture_coordinates_bo = model->m_texture_coordinates_bo;
        model2->m_vaob = model->m_vaob;
        m_models[modelId] = model2;
        return model2;
    }

    void ModelManager::disableModel(unsigned int modelId)
    {
        if (m_models.find(modelId) != m_models.end()) 
        {
            m_models[modelId]->m_enabled = false;
        }
    }

    void ModelManager::destroy()
    {
        for (auto it : m_models)
        {
            freeModel(it.second);
        }
    }
}