#include "SeparableBlur.h"

namespace engine
{
    SeparableBlur::SeparableBlur(GLuint downsampleShader, GLuint hBlurShader, GLuint vBlurShader, int width, int height) : 
        PostFx(downsampleShader, width, height), m_vBlurShader(vBlurShader), m_hBlurShader(hBlurShader)
    {
        m_hBlurFbo = new engine::FboInfo(width, height, 1);
        m_vBlurFbo = new engine::FboInfo(width, height, 1);
    }

    void SeparableBlur::render(engine::FboInfo* source)
    {
        engine::FboInfo* downsampled = downsample(source, 2);
        separableBlur(m_hBlurShader, m_vBlurShader, downsampled, m_hBlurFbo, m_vBlurFbo);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_vBlurFbo->getColorTextureTarget(0));
    }

    void SeparableBlur::setShaderValues(GLuint program)
    {
    }

    void SeparableBlur::destroy()
    {
        m_hBlurFbo->destroy();
        m_vBlurFbo->destroy();
        delete m_hBlurFbo;
        delete m_vBlurFbo;
        PostFx::destroy();
    }
}