#include "FboInfo.h"
#include "Helper.h"

namespace engine
{
    FboInfo::FboInfo() : m_isComplete(false)
        , m_frameBufferId(UINT32_MAX)
        , m_depthBuffer(UINT32_MAX)
        , m_width(0)
        , m_height(0)
    {

    };

    FboInfo::FboInfo(int w, int h, uint16_t colorTextureTargets)
    {
        m_isComplete = false;
        m_width = w;
        m_height = h;
        // Generate textures and set filter parameters (no storage allocated yet)
        for (int i = 0; i < colorTextureTargets; i++)
        {
            GLuint colorTextureTarget;
            glGenTextures(1, &colorTextureTarget);
            glBindTexture(GL_TEXTURE_2D, colorTextureTarget);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            m_colorTextureTargets.push_back(colorTextureTarget);
        }

        glGenTextures(1, &m_depthBuffer);
        glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // allocate storage for textures
        resize(m_width, m_height);

        ///////////////////////////////////////////////////////////////////////
        // Generate and bind framebuffer
        ///////////////////////////////////////////////////////////////////////
        glGenFramebuffers(1, &m_frameBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
        // bind the texture as color attachment (to the currently bound framebuffer)
        std::vector<GLenum> drawBuffers;
        for (unsigned int i = 0; i < m_colorTextureTargets.size(); i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorTextureTargets[i], 0);
            drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        }
        glDrawBuffers(drawBuffers.size(), &drawBuffers[0]);

        // bind the texture as depth attachment (to the currently bound framebuffer)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);

        // check if framebuffer is complete
        m_isComplete = checkFramebufferComplete();

        // bind default framebuffer, just in case.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FboInfo::destroy()
    {
        glDeleteTextures(m_colorTextureTargets.size(), &m_colorTextureTargets[0]);
        glDeleteTextures(1, &m_depthBuffer);
        glDeleteFramebuffers(1, &m_frameBufferId);
    }

    void FboInfo::resize(int w, int h)
    {
        m_width = w;
        m_height = h;
        // Allocate a texture
        for (const auto& colorTextureTarget : m_colorTextureTargets)
        {
            glBindTexture(GL_TEXTURE_2D, colorTextureTarget);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        // generate a depth texture
        glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }

    bool FboInfo::checkFramebufferComplete(void) {
        // Check that our FBO is correctly set up, this can fail if we have
        // incompatible formats in a buffer, or for example if we specify an
        // invalid drawbuffer, among things.
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            engine::fatal_error("Framebuffer not complete");
        }

        return (status == GL_FRAMEBUFFER_COMPLETE);
    }

    GLuint FboInfo::getFrameBufferId() const
    {
        return m_frameBufferId;
    }

    GLuint FboInfo::getColorTextureTarget(unsigned int position) const
    {
        if (position > m_colorTextureTargets.size() - 1)
        {
            return UINT32_MAX;
        }
        return m_colorTextureTargets[position];
    }

    GLuint FboInfo::getDepthBuffer() const
    {
        return m_depthBuffer;
    }

    int FboInfo::getWidth() const
    {
        return m_width;
    }

    int FboInfo::getHeight() const
    {
        return m_height;
    }

    unsigned int FboInfo::getColorTextureTargetsSize() const
    {
        return m_colorTextureTargets.size();
    }
}