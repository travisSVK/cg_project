#pragma once
#include "collision/BoundingBox.h"

namespace engine
{
    class Camera;
    class Model;
}

class CameraBoundingBox : public engine::BoundingBox 
{
public:
    CameraBoundingBox(glm::vec3 dimensions, float rotationAngle, engine::Model* gameCameraModel, engine::Camera* camera);
    virtual void notifyCollision(engine::ColliderType other);
    virtual void getColliderInfo(std::vector<glm::vec3>& edges, std::vector<glm::vec3>& vertices);

private:
    engine::Camera* m_camera;
    engine::Model* m_gameCameraModel;
};