#pragma once
#include <GL/glew.h>

class PostFX
{
public:
    void separableBlur(GLuint hShader, GLuint vShader, const FboInfo &source, const FboInfo &hBlurFbo, const FboInfo &vBlurFbo);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, hBlurFbo.framebufferId);
        glUseProgram(hShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source.colorTextureTarget);
        labhelper::drawFullScreenQuad();

        // vertical blur
        glBindFramebuffer(GL_FRAMEBUFFER, vBlurFbo.framebufferId);
        glUseProgram(vShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hBlurFbo.colorTextureTarget);
        labhelper::drawFullScreenQuad();
    }
};