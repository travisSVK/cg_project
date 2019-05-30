#include "ParticleSystem.h"
#include <GL/glew.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>

#include <imgui.h>


#include <glm/gtx/transform.hpp>
#include <iostream>
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include <stb_image.h>
namespace engine
{
    // OpenGL Project - 12/03/2018 - 15.57 - Vec4 needed to store the data. xyz is the view space position and w is the lifetime.
    glm::vec4 Test;
    ParticleSystem::ParticleSystem()
    {}

    ParticleSystem::ParticleSystem(int size)
    {
        max_size = size;
    }
    // OpenGL Project - 12/03/2018 - 15.58 - Creates a particle and adds it at the end of the ParticleSystem vector with vector.push_back()
    void ParticleSystem::spawn(Particle particle, glm::mat4 jet)
    {
        for (int i = 0; i < 1; i++)
        {
            const float theta = engine::uniform_randf(0.f, 2.f * M_PI);
            //fountain effect
            //const float u = labhelper::uniform_randf(-1.f, 1.f);
            //cone/cirlce effect
            const float u = engine::uniform_randf(0.95f, 1.f);
            //particle.pos = { 0.0,0.0,0.0,1.0 };
            particle.pos = glm::vec3(jet * glm::vec4{ 23.0f,19.5f,0.0f,1.0f });
            //particle.velocity = 15.0f * glm::vec3(sqrt(1.f - u * u) * cosf(theta), u, sqrt(1.f - u * u) * sinf(theta));
            particle.velocity = glm::mat3{ jet } *(10.0f * glm::vec3(sqrt(1.f - u * u) * cosf(theta), u*2.0f, sqrt(1.f - u * u) * sinf(theta)));
            //particle.velocity = glm::vec3(0.0f,1.0f,0.0f);
            particle.life_length = 1.0f;
            particle.lifetime = 0;
            if (particles.size() < max_size)
            {
                particles.push_back(particle);
            }
        }
        //return;
    }

    // OpenGL Project - 12/03/2018 - 15.58 - Process Particles by calling process_particles()
    void ParticleSystem::process_particles(float dt)
    {
        // OpenGL Project - 12/03/2018 - 16.02 - Kill dead particles! i is executed N times. 
        // N equals to the number of particles stored in the ParticleSystem vector.
        for (unsigned i = 0; i < particles.size(); i++)
        {
            // OpenGL Project - 12/03/2018 - 16.03 - For each id.lifetime > id.life_length kill id particle
            if (particles[i].lifetime > particles[i].life_length)
            {
                kill(i);
            }
        }
        // OpenGL Project - 12/03/2018 - 16.02 - Update alive particles! i is executed N times. 
        // N equals to the number of particles stored in the ParticleSystem vector.
        for (unsigned i = 0; i < particles.size(); i++)
        {
            // OpenGL Project - 12/03/2018 - 16.03 - Update particles[i].lifetime
            particles[i].lifetime = particles[i].lifetime + dt;
            // OpenGL Project - 12/03/2018 - 16.03 - Update particles[i].pos
            glm::vec3 tmpVelocity = particles[i].velocity;
            particles[i].pos = particles[i].pos + (tmpVelocity * dt);
        }
        return;
    }

    // OpenGL Project - 12/03/2018 - 15.58 - Kills particle number "id"
    void ParticleSystem::kill(int id)
    {
        // OpenGL Project - 12/03/2018 - 15.58 - Kills particle number "id"
        particles[id] = particles.back();
        particles.pop_back();
        return;
    }
    void ParticleSystem::init_particle_system(ParticleSystem* particles_system)
    {
        glGenVertexArrays(1, &vertexArrayPart);
        glBindVertexArray(vertexArrayPart);
        ///////////////////////////////////////////////////////////////////////
        //		Particle Buffers
        ///////////////////////////////////////////////////////////////////////
        glGenBuffers(1, &particleBuffer);
        // Set the newly created buffer as the current one
        glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);
        glBufferData(GL_ARRAY_BUFFER, particles_system->max_size * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW); //??
        glVertexAttribPointer(0, 4, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
        glEnableVertexAttribArray(0);

        particleShaderProgram = engine::loadShaderProgram("shaders/particle.vert", "shaders/particle.frag", "", "");

        //***********************************************************************
        //			Load Texture
        //***********************************************************************
        int wExp, hExp, compExp;
        unsigned char* imageExp = stbi_load("../scenes/smokerinoPagourino.png", &wExp, &hExp, &compExp, STBI_rgb_alpha);
        // Apparently, we need to tell OpenGL what to do with texture coordinates outside the (0,1) range.

        // Then we can generate a OpenGL texture and initialize it with the texture data. 
        // Textures are identified by an integer in OpenGL, and we can generate such an identifier with: 
        glGenTextures(1, &vertexArrayObjectExp);
        // Then we can bind this texture, allocate storage and upload the data: 
        glBindTexture(GL_TEXTURE_2D, vertexArrayObjectExp);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wExp, hExp, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageExp);
        free(imageExp);
        // We can define where to sample when a coordinate is outside this range, 
        // and we start by specifying OpenGL to clamp the texture coordinate to this range.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenerateMipmap(GL_TEXTURE_2D);
        // Sets the type of filtering to be used on magnifying and
        // minifying the active texture. These are the nicest available options.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // Make it less noisy. enables an extension
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
    }

    void ParticleSystem::render(ParticleSystem* particles_system, glm::mat4 houseModelMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, int m_width, int m_height, float deltaTime)
    {
        
        engine::Particle part;
        part.pos = { 0.0f, 10.0f, 0.0f };
        part.velocity = { 0.0f, 0.0f, 0.0f };
        part.life_length = 0;
        part.lifetime = 0;
        particles_system->spawn(part, houseModelMatrix);

        //2. Process particles by calling process_particles()
        particles_system->process_particles(deltaTime);

        //extract the data
        
        unsigned int active_particles = particles_system->particles.size();
        data.clear();
        for (int i = 0; i < particles_system->particles.size(); i++)
        {
            glm::vec4 temp = { glm::vec3((viewMatrix)* glm::vec4(particles_system->particles.at(i).pos,1)), particles_system->particles.at(i).lifetime };
            //transform into viewspace
            data.push_back(temp);
        }

        std::sort(data.begin(), std::next(data.begin(), active_particles),
            [](const glm::vec4 &lhs, const glm::vec4 &rhs) { return lhs.z < rhs.z; });
        glBindVertexArray(particles_system->vertexArrayPart);
        glBindBuffer(GL_ARRAY_BUFFER, particles_system->particleBuffer);
        //before 8 upload the data to gpu
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * active_particles, &data[0]);
        engine::setUniformSlow(particles_system->particleShaderProgram, "P", projectionMatrix);
        engine::setUniformSlow(particles_system->particleShaderProgram, "screen_x", float(m_width));
        engine::setUniformSlow(particles_system->particleShaderProgram, "screen_y", float(m_height));

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        // Enable shader program point size modulation.
        glEnable(GL_PROGRAM_POINT_SIZE);
        // Enable blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, particles_system->vertexArrayObjectExp);

        glDrawArrays(GL_POINTS, 0, active_particles);

        glDisable(GL_PROGRAM_POINT_SIZE);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }

    void ParticleSystem::destroy()
    {
        glDeleteProgram(particleShaderProgram);
    }

    GLuint ParticleSystem::useProgram()
    {
        glUseProgram(particleShaderProgram);
        return particleShaderProgram;
    }
}