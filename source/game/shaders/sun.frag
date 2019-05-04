#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

in vec2 texCoord;

layout(binding = 0) uniform sampler2D colortexture;
layout(location = 0) out vec4 fragmentColor;

void main() 
{ 
    fragmentColor = texture2D(colortexture, texCoord.xy);
    //fragmentColor = vec4(0.0, 0.0, 0.0, 1.0);
}