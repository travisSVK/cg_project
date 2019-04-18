#include "Bloom.h"
#include "../Helper.h"

namespace engine
{
    Bloom::Bloom(GLuint downsampleShader, GLuint hBlurShader, GLuint vBlurShader, int width, int height) : 
        PostFx(downsampleShader, width, height), m_vBlurShader(vBlurShader), m_hBlurShader(hBlurShader)
    {
        m_cutoffFbo = new engine::FboInfo(width, height, 1);
        m_hBlurFbo = new engine::FboInfo(width, height, 1);
        m_vBlurFbo = new engine::FboInfo(width, height, 1);
        m_cutoffShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/cutoff.frag", "", "");
    }

    void Bloom::render(engine::FboInfo* source)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_cutoffFbo->getFrameBufferId());
        glUseProgram(m_cutoffShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source->getColorTextureTarget(0));
        engine::drawFullScreenQuad();

        engine::FboInfo* downsampled = downsample(m_cutoffFbo, 2);
        separableBlur(m_hBlurShader, m_vBlurShader, downsampled, m_hBlurFbo, m_vBlurFbo);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source->getColorTextureTarget(0));
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, m_vBlurFbo->getColorTextureTarget(0));
    }

    void Bloom::setShaderValues(GLuint program)
    {
    }

    void Bloom::destroy()
    {
        m_cutoffFbo->destroy();
        m_hBlurFbo->destroy();
        m_vBlurFbo->destroy();
        delete m_cutoffFbo;
        delete m_hBlurFbo;
        delete m_vBlurFbo;
        glDeleteProgram(m_cutoffShader);
        PostFx::destroy();
    }
}