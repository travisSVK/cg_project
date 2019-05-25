#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(binding = 0) uniform sampler2D colortexture;
layout(location = 0) out vec4 fragmentColor;

uniform float flashTime;
uniform vec3 color;

void main() 
{ 
    fragmentColor = vec4(color, flashTime);
}