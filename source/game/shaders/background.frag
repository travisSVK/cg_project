#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(location = 0) out vec4 fragmentColor;
layout (binding = 6) uniform sampler2D environmentMap; 
in vec2 texCoord; 
uniform mat4 inv_PV;
uniform vec3 camera_pos;
uniform float environment_multiplier;
#define PI 3.14159265359

///////////////////////////////////////////////////////////////////////////////
// DoF values
///////////////////////////////////////////////////////////////////////////////

uniform float nearSharpPlane;
uniform float farSharpPlane;
uniform float nearBlurryPlane;
uniform float farBlurryPlane;
uniform int farCoC;
uniform int nearCoC;
uniform float focusCoC;

float calculateCoCRadius(float worldZ)
{
    // default value for focus CoC
    float CoCRadius = focusCoC;
    // we are in the near blurry plane
    if(worldZ > nearSharpPlane)
    {
        CoCRadius = nearCoC;
        if(worldZ < nearBlurryPlane)
        {
            CoCRadius = ((worldZ - nearSharpPlane) / (nearBlurryPlane - nearSharpPlane)) * float(nearCoC - focusCoC);
        }
    }
    // we are in the far blurry plane
    else if(worldZ < farSharpPlane)
    {
        CoCRadius = farCoC;
        if(worldZ > farBlurryPlane)
        {
            CoCRadius = ((farSharpPlane - worldZ) / (farSharpPlane - farBlurryPlane)) * float(farCoC - focusCoC);
        }
    }

    return float(CoCRadius);
}

void main() 
{
	// Calculate the world-space position of this fragment on the near plane
	vec4 pixel_world_pos = inv_PV * vec4(texCoord * 2.0 - 1.0, 1.0, 1.0);
	pixel_world_pos = (1.0 / pixel_world_pos.w) * pixel_world_pos;
	// Calculate the world-space direction from the camera to that position
	vec3 dir = normalize(pixel_world_pos.xyz - camera_pos);
	// Calculate the spherical coordinates of the direction
	float theta = acos(max(-1.0f, min(1.0f, dir.y)));
	float phi = atan(dir.z, dir.x);
	if (phi < 0.0f) phi = phi + 2.0f * PI;
	// Use these to lookup the color in the environment map
	vec2 lookup = vec2(phi / (2.0 * PI), theta / PI);
	fragmentColor = environment_multiplier * texture(environmentMap, lookup);
    fragmentColor = vec4(fragmentColor.xyz, calculateCoCRadius(pixel_world_pos.z));
}


