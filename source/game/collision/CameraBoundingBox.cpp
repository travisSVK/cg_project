#include "CameraBoundingBox.h"
#include "scene/Camera.h"
#include "model/Model.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

CameraBoundingBox::CameraBoundingBox(glm::vec3 dimensions, float rotationAngle, engine::Model* gameCameraModel, engine::Camera* camera) :
    engine::BoundingBox(dimensions, rotationAngle, gameCameraModel), m_camera(camera)
{}

void CameraBoundingBox::notifyCollision(engine::ColliderType other)
{
    // reset camera position
    m_camera->resetPosition();
    // reset position for game model in parent
    engine::BoundingBox::notifyCollision(other);
}

void CameraBoundingBox::getColliderInfo(std::vector<glm::vec3>& edges, std::vector<glm::vec3>& vertices)
{
    // first, compute world coordinates of vertices
    glm::mat4 modelMatrix = glm::translate(m_camera->getPosition());
    vertices = m_vertices;
    for (auto& vertex : vertices)
    {
        vertex = glm::vec3(modelMatrix * glm::vec4(vertex, 1.0f));
    }

    // get edges - basically get obb normals
    edges.push_back(vertices[1] - vertices[0]);
    edges.push_back(vertices[2] - vertices[1]);
    edges.push_back(vertices[4] - vertices[0]);
}