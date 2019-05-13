#include "SATCollisionStrategy.h"
#include <glm/glm.hpp>
#include <numeric>

namespace engine
{
    bool SATCollisionStrategy::checkCollision(ICollider* first, ICollider* second)
    {
        std::vector<glm::vec3> edgesFirst;
        std::vector<glm::vec3> verticesFirst;
        first->getColliderInfo(edgesFirst, verticesFirst);
        std::vector<glm::vec3> edgesSecond;
        std::vector<glm::vec3> verticesSecond;
        second->getColliderInfo(edgesSecond, verticesSecond);
        
        if (!detectCollision(edgesFirst, verticesFirst, verticesSecond))
        {
            return false;
        }
        if (!detectCollision(edgesSecond, verticesSecond, verticesFirst))
        {
            return false;
        }
        // make cross product edges
        std::vector<glm::vec3> edges;
        edges.push_back(glm::cross(edgesFirst[0], edgesSecond[0]));
        edges.push_back(glm::cross(edgesFirst[0], edgesSecond[1]));
        edges.push_back(glm::cross(edgesFirst[0], edgesSecond[2]));
        edges.push_back(glm::cross(edgesFirst[1], edgesSecond[0]));
        edges.push_back(glm::cross(edgesFirst[1], edgesSecond[1]));
        edges.push_back(glm::cross(edgesFirst[1], edgesSecond[2]));
        edges.push_back(glm::cross(edgesFirst[2], edgesSecond[0]));
        edges.push_back(glm::cross(edgesFirst[2], edgesSecond[1]));
        edges.push_back(glm::cross(edgesFirst[2], edgesSecond[2]));
        return detectCollision(edges, verticesFirst, verticesSecond);
    }
    bool SATCollisionStrategy::detectCollision(const std::vector<glm::vec3>& edges,
        const std::vector<glm::vec3>& verticesFirst, const std::vector<glm::vec3>& verticesSecond)
    {
        for (const auto& edgeFirst : edges) {
            glm::vec3 axis;
            axis.x = edgeFirst.x;
            axis.y = edgeFirst.y;
            axis.z = edgeFirst.z;
            float polygonAMax = -std::numeric_limits<float>::infinity();
            float polygonAMin = std::numeric_limits<float>::infinity();
            float polygonBMax = -std::numeric_limits<float>::infinity();
            float polygonBMin = std::numeric_limits<float>::infinity();
            for (const auto& vertex : verticesFirst) {
                std::vector<float> axisVec{ axis.x, axis.y, axis.z };
                std::vector<float> vertexVec{ vertex.x, vertex.y, vertex.z };
                auto proj = std::inner_product(std::begin(axisVec), std::end(axisVec), std::begin(vertexVec), 0.0);
                if (proj < polygonAMin) polygonAMin = proj;
                if (proj > polygonAMax) polygonAMax = proj;
            }
            for (const auto& vertex : verticesSecond) {
                std::vector<float> axisVec{ axis.x, axis.y, axis.z };
                std::vector<float> vertexVec{ vertex.x, vertex.y, vertex.z };
                auto proj = std::inner_product(std::begin(axisVec), std::end(axisVec), std::begin(vertexVec), 0.0);
                if (proj < polygonBMin) polygonBMin = proj;
                if (proj > polygonBMax) polygonBMax = proj;
            }
            if (polygonAMax < polygonBMin || polygonAMin > polygonBMax)
            {
                return false;
            }
        }
        return true;
    }
}