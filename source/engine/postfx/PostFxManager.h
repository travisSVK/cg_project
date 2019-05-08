#pragma once
#include <GL/glew.h>
#include <unordered_map>
#include <glm/glm.hpp>

namespace engine
{
    class PostFx;
    class FboInfo;
    class PostFxManager
    {
    public:

        enum class PostFxTypes : unsigned int
        {
            None = 0,
            Sepia = 1,
            Mushroom = 2,
            Blur = 3,
            Grayscale = 4,
            Composition = 5,
            Mosaic = 6,
            Separable_blur = 7,
            Bloom = 8,
            Motion_Blur = 9,
            DOF = 10,
            Pseudo_Lens_Flare = 11,
            Lens_Flare = 12
        };

        PostFxManager(int width, int height);
        void renderPostFx(PostFxTypes type, engine::FboInfo* source, engine::FboInfo* to, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
        void renderPostFxToMain(PostFxTypes type, engine::FboInfo* source, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
        //void renderPostFx(PostFxTypes type, engine::FboInfo* source, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
        void setShaderValues(PostFxTypes type, GLuint program);
        void destroy();

    private:
        std::unordered_map<PostFxTypes, PostFx*> m_effects;
        std::vector<GLuint> m_programs;
        GLuint m_postfxProgram;
        int m_width;
        int m_height;
        glm::mat4 m_previousViewProjectionMat;
    };
}