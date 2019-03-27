#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

///////////////////////////////////////////////////////////////////////////////
// Material
///////////////////////////////////////////////////////////////////////////////
uniform vec3 material_color;
uniform float material_reflectivity;
uniform float material_metalness;
uniform float material_fresnel;
uniform float material_shininess;
uniform float material_emission;

uniform int has_emission_texture;
layout(binding = 5) uniform sampler2D emissiveMap;

///////////////////////////////////////////////////////////////////////////////
// Environment
///////////////////////////////////////////////////////////////////////////////
layout(binding = 6) uniform sampler2D environmentMap;
layout(binding = 7) uniform sampler2D irradianceMap;
layout(binding = 8) uniform sampler2D reflectionMap;
uniform float environment_multiplier;

///////////////////////////////////////////////////////////////////////////////
// DoF values
///////////////////////////////////////////////////////////////////////////////

uniform float nearSharpPlane;
uniform float farSharpPlane;
uniform float nearBlurryPlane;
uniform float farBlurryPlane;
uniform int farCoC;
uniform int nearCoC;
uniform int focusCoC;
in float worldZ;

///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
uniform vec3 point_light_color = vec3(1.0, 1.0, 1.0);
uniform float point_light_intensity_multiplier = 50.0;

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////
#define PI 3.14159265359

///////////////////////////////////////////////////////////////////////////////
// Input varyings from vertex shader
///////////////////////////////////////////////////////////////////////////////
in vec2 texCoord;
in vec3 viewSpaceNormal;
in vec3 viewSpacePosition;

///////////////////////////////////////////////////////////////////////////////
// Input uniform variables
///////////////////////////////////////////////////////////////////////////////
uniform mat4 viewInverse;
uniform vec3 viewSpaceLightPosition;

///////////////////////////////////////////////////////////////////////////////
// Output color
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) out vec4 fragmentColor;

vec3 calculateDirectIllumiunation(vec3 wo, vec3 n)
{
	///////////////////////////////////////////////////////////////////////////
	// Task 1.2 - Calculate the radiance Li from the light, and the direction
	//            to the light. If the light is backfacing the triangle, 
	//            return vec3(0); 
	///////////////////////////////////////////////////////////////////////////
    float d = distance(viewSpacePosition, viewSpaceLightPosition);
    vec3 li = point_light_intensity_multiplier * point_light_color * 1 / (d * d);

	///////////////////////////////////////////////////////////////////////////
	// Task 1.3 - Calculate the diffuse term and return that as the result
	///////////////////////////////////////////////////////////////////////////
    vec3 wi = normalize(viewSpaceLightPosition - viewSpacePosition);
    if (dot(n, wi) <= 0) {
        return vec3(0.0);
    }
    
    vec3 diffuse_term = material_color * (1.0 / PI) * abs(dot(n, wi)) * li;

	///////////////////////////////////////////////////////////////////////////
	// Task 2 - Calculate the Torrance Sparrow BRDF and return the light 
	//          reflected from that instead
	///////////////////////////////////////////////////////////////////////////
    vec3 wh = normalize(wi + wo);
    float s = material_shininess;

    float fresnel = material_fresnel + (1 - material_fresnel) * pow((1 - dot(wh, wi)), 5);    
    float microfacet_distribution = ((s + 2) / 2 * PI) * pow(dot(n, wh), s);
    float shadowing = min(1, min(2 * ((dot(n, wh) * dot(n, wo)) / dot(wo, wh)), 2 * ((dot(n, wh) * dot(n, wi)) / dot(wo, wh))));
    float brdf = (fresnel * microfacet_distribution * shadowing) / (4 * dot(n, wo) * dot(n, wi));

    ///////////////////////////////////////////////////////////////////////////
	// Task 3 - Make your shader respect the parameters of our material model.
	///////////////////////////////////////////////////////////////////////////
	vec3 dielectric_term = brdf * (dot(n, wi)) * li + (1 - fresnel) * diffuse_term;
    vec3 metal_term = brdf * material_color * (dot(n, wi)) * li;
    vec3 microfacet_term = material_metalness * metal_term + (1 - material_metalness) * dielectric_term;

    return material_reflectivity * microfacet_term + (1 - material_reflectivity) * diffuse_term;
    //return brdf * dot(n, wi) * li; //vec3(diffuse_term);
}

vec3 calculateIndirectIllumination(vec3 wo, vec3 n)
{
	///////////////////////////////////////////////////////////////////////////
	// Task 5 - Lookup the irradiance from the irradiance map and calculate
	//          the diffuse reflection
	///////////////////////////////////////////////////////////////////////////
    vec3 nws = normalize(mat3(viewInverse) * n);
    
    float theta = acos(max(-1.0f, min(1.0f, nws.y)));
	float phi = atan(nws.z, nws.x);
	if (phi < 0.0f) phi = phi + 2.0f * PI;
    vec2 lookup = vec2(phi / (2.0 * PI), theta / PI);
    // Dont know if the environment_multiplier
    vec3 diffuse_term = material_color * (1.0 / PI) * environment_multiplier * texture(irradianceMap, lookup).rgb;

    ///////////////////////////////////////////////////////////////////////////
	// Task 6 - Look up in the reflection map from the perfect specular 
	//          direction and calculate the dielectric and metal terms. 
	/////////////////////////////////////////////////////////////////////////
    
    vec3 wi = normalize(reflect(-wo, n));

    theta = acos(max(-1.0f, min(1.0f, wi.y)));
	phi = atan(wi.z, wi.x);
	if (phi < 0.0f) phi = phi + 2.0f * PI;
    lookup = vec2(phi / (2.0 * PI), theta / PI);
    float roughness = sqrt(sqrt(2 / (material_shininess + 2)));
    vec3 li = environment_multiplier * textureLod(reflectionMap, lookup, roughness * 7.0).xyz;

    vec3 wh = normalize(wi + wo);
    float fresnel = material_fresnel + (1 - material_fresnel) * pow((1 - dot(wh, wi)), 5);    
    // float shadowing = min(1, min(2 * ((dot(n, wh) * dot(n, wo)) / dot(wo, wh)), 2 * ((dot(n, wh) * dot(n, wi)) / dot(wo, wh))));
    vec3 dielectric_term = fresnel * li + (1 - fresnel) * diffuse_term;
    vec3 metal_term = fresnel * material_color * li;

    vec3 microfacet_term = material_metalness * metal_term + (1 - material_metalness) * dielectric_term;
    return material_reflectivity * microfacet_term + (1 - material_reflectivity) * diffuse_term;
}

float calculateCoCRadius()
{
    // default value for focus CoC
    int CoCRadius = focusCoC;
    // we are in the near blurry plane
    if(worldZ > nearSharpPlane)
    {
        CoCRadius = nearCoC;
        // closer than near blurry plane, just clamp to max CoC for near blurry plane
//        if(worldZ > nearBlurryPlane)
//        {
//            CoCRadius = nearCoC;
//        }
//        // else interpolate CoC value between near blurry and near sharp CoC values
//        else
//        {
//            CoCRadius = int(((worldZ - nearSharpPlane) / (nearBlurryPlane - nearSharpPlane)) * (nearCoC - focusCoC));
//        }
    }
    // we are in the far blurry plane
    else if(worldZ < farSharpPlane)
    {
        CoCRadius = farCoC;
        // farther than far blurry plane, just clamp to max CoC for far blurry plane
//        if(worldZ < farBlurryPlane)
//        {
//            CoCRadius = farCoC;
//        }
//        // else interpolate CoC value between far blurry and far sharp CoC values
//        else
//        {
//            CoCRadius = int(abs((farSharpPlane - worldZ) / (farSharpPlane - farBlurryPlane)) * (farCoC - focusCoC));
//        }
    }

    return float(CoCRadius);
}

void main()
{
	vec3 wo = -normalize(viewSpacePosition);
	vec3 n = normalize(viewSpaceNormal);

	// Direct illumination
	vec3 direct_illumination_term = calculateDirectIllumiunation(wo, n);

	// Indirect illumination
	vec3 indirect_illumination_term = calculateIndirectIllumination(wo, n);

	///////////////////////////////////////////////////////////////////////////
	// Add emissive term. If emissive texture exists, sample this term.
	///////////////////////////////////////////////////////////////////////////
	vec3 emission_term = material_emission * material_color;
	if (has_emission_texture == 1) {
		emission_term = texture(emissiveMap, texCoord).xyz;
	}

	fragmentColor.xyz =
		direct_illumination_term +
		indirect_illumination_term +
		emission_term;

    fragmentColor = vec4(fragmentColor.xyz, calculateCoCRadius());
}
