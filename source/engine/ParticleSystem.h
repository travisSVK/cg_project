#pragma once
#include <GL/glew.h>
#include <vector>
#include <glm/detail/type_vec3.hpp>
#include <glm/mat4x4.hpp>
#include"Helper.h"
#define GLM_ENABLE_EXPERIMENTAL

namespace engine
{
    struct Particle 
    {
        float lifetime;
        float life_length;
        glm::vec3 velocity;
        glm::vec3 pos;
    };

    class ParticleSystem 
    {
    public:
        // Members
        std::vector<Particle> particles;
        int max_size;
        // Ctor/Dtor
        ParticleSystem();
        ParticleSystem(int size);
        ~ParticleSystem() {}
        // Methods
        void spawn(Particle particle, glm::mat4);
        void process_particles(float dt);
        void kill(int id);
        void init_particle_system(ParticleSystem* particles_system);
        void render(ParticleSystem* particles_system, glm::mat4 houseModelMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix,  int m_width, int m_height, float deltaTime);
        void destroy();
        GLuint useProgram();
        Particle particle;
        GLuint 	particleBuffer;
        GLuint particleShaderProgram, vertexArrayObjectExp, vertexArrayPart;
        std::vector<glm::vec4> data;
    };
}
