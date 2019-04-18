#pragma once
#include <GL/glew.h>
#include <vector>

namespace engine 
{
    class FboInfo
    {
    public:
        FboInfo(int w, int h, uint16_t colorTextureTargets);
        FboInfo();
        void resize(int w, int h);
        GLuint getFrameBufferId() const;
        GLuint getColorTextureTarget(unsigned int position) const;
        unsigned int getColorTextureTargetsSize() const;
        GLuint getDepthBuffer() const;
        int getWidth() const;
        int getHeight() const;
        void destroy();

    private:
        bool checkFramebufferComplete(void);

    private:
        GLuint m_frameBufferId;
        std::vector<GLuint> m_colorTextureTargets;
        GLuint m_depthBuffer;
        int m_width;
        int m_height;
        bool m_isComplete;
    };
}