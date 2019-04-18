#pragma once
#include "PostFx.h"

namespace engine
{
    class Bloom : public PostFx 
    {
    public:

        Bloom(GLuint downsampleShader, GLuint hBlurShader, GLuint vBlurShader, int width, int height);
        virtual void render(engine::FboInfo* source) override;
        virtual void destroy() override;
        virtual void setShaderValues(GLuint program) override;

    private:

        engine::FboInfo* m_cutoffFbo;
        engine::FboInfo* m_hBlurFbo;
        engine::FboInfo* m_vBlurFbo;
        GLuint m_cutoffShader;
        GLuint m_hBlurShader;
        GLuint m_vBlurShader;
    };
}