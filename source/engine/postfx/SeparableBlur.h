#pragma once
#include "PostFx.h"

namespace engine
{
    class SeparableBlur : public PostFx
    {
    public:

        SeparableBlur(GLuint downsampleShader, GLuint hBlurShader, GLuint vBlurShader, int width, int height);
        virtual void render(engine::FboInfo* source) override;
        virtual void destroy() override;
        virtual void setShaderValues(GLuint program) override;

    private:

        engine::FboInfo *m_hBlurFbo;
        engine::FboInfo *m_vBlurFbo;
        GLuint m_hBlurShader;
        GLuint m_vBlurShader;
    };
}