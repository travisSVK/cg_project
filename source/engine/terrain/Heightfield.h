#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace engine
{
    class Sun;
    class HeightField {
    public:

        HeightField();

        // load height field
        void loadHeightField(const std::string &heigtFieldPath);

        // load diffuse map
        void loadDiffuseTexture(const std::string &diffusePath);

        // generate mesh
        // @param tesselation [in] Number of triangles per side.
        void generateMesh(int tesselation);

        // render height map
        void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& cameraWorldPos, float environmentMultiplier, Sun* sun);

        GLuint useProgram();

    private:

        int m_meshResolution; // triangles edges per quad side
        GLuint m_texid_hf;
        GLuint m_texid_diffuse;
        GLuint m_vao;
        GLuint m_positionBuffer;
        GLuint m_uvBuffer;
        GLuint m_indexBuffer;
        GLuint m_numIndices;
        GLuint m_shader;
        std::string m_heightFieldPath;
        std::string m_diffuseTexturePath;
    };
}