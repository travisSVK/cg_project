#pragma once
#include "../FboInfo.h"
#include <GL/glew.h>

namespace engine
{
    class PostFx
    {
    public:

        PostFx(GLuint downsampleShader, int width, int height);
        virtual ~PostFx() {}
        // render from input fbo
        virtual void render(engine::FboInfo* source) = 0;
        virtual void setShaderValues(GLuint program) = 0;
        virtual void destroy();

    protected:
        engine::FboInfo* downsample(engine::FboInfo *source, unsigned int level);
        void separableBlur(GLuint hShader, GLuint vShader, engine::FboInfo *source, engine::FboInfo *hBlurFbo, engine::FboInfo *vBlurFbo);
    
    private:
        GLuint m_downsampleShader;
        engine::FboInfo *m_downsampleFbo;
        int m_width;
        int m_height;
    };
}