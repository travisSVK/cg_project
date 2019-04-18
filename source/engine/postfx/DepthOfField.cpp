#include "DepthOfField.h"
#include "../Helper.h"

namespace engine
{
    DepthOfField::DepthOfField(GLuint downsampleShader, GLuint hBlurShader, GLuint vBlurShader, int width, int height):
        PostFx(downsampleShader, width, height), m_vBlurShader(vBlurShader), m_hBlurShader(hBlurShader)
    {
        m_hBlurFbo = new engine::FboInfo(width, height, 2);
        m_vBlurFbo = new engine::FboInfo(width, height, 2);
    }

    void DepthOfField::render(engine::FboInfo* source)
    {
        engine::FboInfo* downsampled = downsample(source, 2);
        separableBlur(m_hBlurShader, m_vBlurShader, downsampled, m_hBlurFbo, m_vBlurFbo);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source->getColorTextureTarget(0));
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, m_vBlurFbo->getColorTextureTarget(0));
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, m_vBlurFbo->getColorTextureTarget(1));
    }

    void DepthOfField::setShaderValues(GLuint program)
    {
        engine::setUniformSlow(program, "nearSharpPlane", 10.0f);
        engine::setUniformSlow(program, "farSharpPlane", -10.0f);
        engine::setUniformSlow(program, "nearBlurryPlane", 70.0f);
        engine::setUniformSlow(program, "farBlurryPlane", -70.0f);
        engine::setUniformSlow(program, "farCoC", -4);
        engine::setUniformSlow(program, "nearCoC", 4);
        engine::setUniformSlow(program, "focusCoC", 0);
    }

    void DepthOfField::destroy()
    {
        m_hBlurFbo->destroy();
        m_vBlurFbo->destroy();
        delete m_hBlurFbo;
        delete m_vBlurFbo;
        PostFx::destroy();
    }
}