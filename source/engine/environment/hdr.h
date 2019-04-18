#pragma once
#include <vector>
#include <string>
#include <GL/glew.h>

namespace engine {

    struct HDRImage {
        int width, height, components;
        float * data = nullptr;
        HDRImage(const std::string & filename);
        ~HDRImage();
    };

	GLuint loadHdrTexture(const std::string &filename);
	GLuint loadHdrMipmapTexture(const std::vector<std::string> &filenames);
}