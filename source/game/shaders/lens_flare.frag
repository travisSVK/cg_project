#version 420

in vec2 pass_textureCoords;

out vec4 out_colour;

layout(binding = 0) uniform sampler2D flareTexture;
layout(binding = 1) uniform sampler2D depthTexture;
uniform float brightness;
uniform vec2 sunPos;

void main(void){

    out_colour = texture(flareTexture, pass_textureCoords);
    out_colour.a *= brightness;
    
    float sceneZ = texture(depthTexture, sunPos).r;
    if(sceneZ < 1.0)
    {
        out_colour.a = 0.0;
    }
}