#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(binding = 0) uniform sampler2D frameBufferTexture;
layout(location = 0) out vec4 fragmentColor;

const float kernel[9] = {
			0.077847, 0.123317, 0.077847,
			0.123317, 0.195346, 0.123317,
			0.077847, 0.123317, 0.077847,
		};

const vec2 offsets[9] = {
			vec2(-1.0, -1.0), vec2( 0.0, -1.0), vec2( 1.0, -1.0),
			vec2(-1.0,  0.0), vec2( 0.0,  0.0), vec2( 1.0,  0.0),
			vec2(-1.0,  1.0), vec2( 0.0,  1.0), vec2( 1.0,  1.0),
		};

vec4 textureRect(in sampler2D tex, vec2 rectangleCoord)
{
	return texture(tex, rectangleCoord * 2 / textureSize(tex, 0));
}

vec4 blur()
{
    vec4 ret = vec4(0.0);
    for (int i = 0; i < 9; ++i) 
    {
		ret += textureRect(frameBufferTexture, gl_FragCoord.xy + offsets[i]) * kernel[i];
	}
    return ret;
}

void main() 
{
    fragmentColor = blur();
}