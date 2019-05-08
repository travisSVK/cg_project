#include "PostFxManager.h"
#include "../Helper.h"
#include "Bloom.h"
#include "DepthOfField.h"
#include "SeparableBlur.h"

namespace engine
{
    PostFxManager::PostFxManager(int width, int height) : m_width(width), m_height(height), m_previousViewProjectionMat(glm::mat4(1.0f))
    {
        m_postfxProgram = engine::loadShaderProgram("shaders/postFx.vert", "shaders/postFx.frag", "", "");

        // create shared shaders here (like downsample and blur)
        GLuint downsampleShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/downsample.frag", "", "");
        GLuint hBlurShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/horizontal_blur.frag", "", "");
        GLuint vBlurShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/vertical_blur.frag", "", "");
        GLuint hDofBlurShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/blur_dof.frag", "", "", "", "#version 420\n#define HORIZONTAL\n");
        GLuint vDofBlurShader = engine::loadShaderProgram("shaders/postFx.vert", "shaders/blur_dof.frag", "", "", "", "#version 420\n");
        m_programs.push_back(downsampleShader);
        m_programs.push_back(hBlurShader);
        m_programs.push_back(vBlurShader);
        m_programs.push_back(hDofBlurShader);
        m_programs.push_back(vDofBlurShader);

        // create effects
        Bloom* bloom = new Bloom(downsampleShader, hBlurShader, vBlurShader, width, height);
        m_effects[PostFxTypes::Bloom] = bloom;
        DepthOfField* depthOfField = new DepthOfField(downsampleShader, hDofBlurShader, vDofBlurShader, width, height);
        m_effects[PostFxTypes::DOF] = depthOfField;
        SeparableBlur* separableBlur = new SeparableBlur(downsampleShader, hBlurShader, vBlurShader, width, height);
        m_effects[PostFxTypes::Separable_blur] = separableBlur;
    }

    void PostFxManager::renderPostFx(PostFxTypes type, engine::FboInfo* source, engine::FboInfo* to, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
    //void PostFxManager::renderPostFx(PostFxTypes type, engine::FboInfo* source, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix)
    {
        if (m_effects.find(type) != m_effects.end())
        {
            m_effects[type]->render(source);
            // get the original viewport resolution in case the effect downsampled
            glViewport(0, 0, m_width, m_height);
        }
        else
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, source->getColorTextureTarget(0));
        }
        glBindFramebuffer(GL_FRAMEBUFFER, to->getFrameBufferId());
        glUseProgram(m_postfxProgram);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, source->getDepthBuffer());
        // engine::setUniformSlow(m_postfxProgram, "time", currentTime); //TODO add time for muschrooms effect
        engine::setUniformSlow(m_postfxProgram, "currentEffect", static_cast<int>(type));
        //engine::setUniformSlow(m_postfxProgram, "filterSize", filterSizes[filterSize - 1]); // TODO add filter size for blur
        engine::setUniformSlow(m_postfxProgram, "viewProjectionInverseMatrix", inverse(projectionMatrix * viewMatrix));
        engine::setUniformSlow(m_postfxProgram, "previousViewProjectionMatrix", m_previousViewProjectionMat);
        engine::setUniformSlow(m_postfxProgram, "numSamples", 5);
        engine::setUniformSlow(m_postfxProgram, "maxCocRadius", 4);
        engine::drawFullScreenQuad();
        m_previousViewProjectionMat = projectionMatrix * viewMatrix;
    }

    void PostFxManager::setShaderValues(PostFxTypes type, GLuint program)
    {
        if (m_effects.find(type) != m_effects.end())
        {
            m_effects[type]->setShaderValues(program);
        }
    }

    void PostFxManager::destroy()
    {
        glDeleteProgram(m_postfxProgram);
        for (auto program : m_programs)
        {
            glDeleteProgram(program);
        }
        for (auto it : m_effects)
        {
            it.second->destroy();
            delete it.second;
        }
    }
}