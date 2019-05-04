#version 420

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoordIn;

out vec2 texCoord;
uniform mat4 viewProjectionMatrix;

void main() 
{
    gl_Position = viewProjectionMatrix * vec4(position, 1.0);
    texCoord = texCoordIn;
}