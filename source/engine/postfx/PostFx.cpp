#include "PostFx.h"
#include "../Helper.h"

namespace engine
{

    PostFx::PostFx(GLuint downsampleShader, int width, int height) : m_downsampleShader(downsampleShader), m_width(width), m_height(height)
    {
        m_downsampleFbo = new engine::FboInfo(width, height, 1);
    }

    engine::FboInfo* PostFx::downsample(engine::FboInfo *source, unsigned int level)
    {
        // downsample
        m_downsampleFbo->resize(m_width / level, m_height / level);
        glViewport(0, 0, m_downsampleFbo->getWidth(), m_downsampleFbo->getHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, m_downsampleFbo->getFrameBufferId());
        glUseProgram(m_downsampleShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source->getColorTextureTarget(0));
        engine::drawFullScreenQuad();

        return m_downsampleFbo;
    }

    void PostFx::separableBlur(GLuint hShader, GLuint vShader, engine::FboInfo *source, engine::FboInfo *hBlurFbo, engine::FboInfo *vBlurFbo)
    {
        // horizontal blur
        hBlurFbo->resize(source->getWidth(), source->getHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, hBlurFbo->getFrameBufferId());
        glUseProgram(hShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_downsampleFbo->getColorTextureTarget(0));
        engine::drawFullScreenQuad();

        // vertical blur
        vBlurFbo->resize(source->getWidth(), source->getHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, vBlurFbo->getFrameBufferId());
        glUseProgram(vShader);
        for (int i = 0; i < hBlurFbo->getColorTextureTargetsSize(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, hBlurFbo->getColorTextureTarget(i));
        }
        engine::drawFullScreenQuad();
    }

    void PostFx::destroy()
    {
        m_downsampleFbo->destroy();
        delete m_downsampleFbo;
    }
}