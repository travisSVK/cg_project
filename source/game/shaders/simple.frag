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

uniform int has_material_color;
uniform int has_texture;
uniform sampler2D material_texture;

uniform sampler2D normalMap;

uniform int has_emission_texture;
layout(binding = 1) uniform sampler2D colorMap;
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
uniform float focusCoC;
in float worldZ;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
uniform vec3 directional_light_color;
uniform float directional_light_intensity_multiplier;

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
uniform mat4 normalMatrix;

///////////////////////////////////////////////////////////////////////////////
// Output color
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) out vec4 fragmentColor;

vec3 calculateDirectIllumiunation(vec3 wo, vec3 n, vec3 materialColor)
{
	///////////////////////////////////////////////////////////////////////////
	// Task 1.2 - Calculate the radiance Li from the light, and the direction
	//            to the light. If the light is backfacing the triangle, 
	//            return vec3(0); 
	///////////////////////////////////////////////////////////////////////////
    float d = distance(viewSpacePosition, viewSpaceLightPosition);
    //vec3 li = point_light_intensity_multiplier * point_light_color * 1 / (d * d);
    vec3 li = directional_light_intensity_multiplier * directional_light_color * 1;

	///////////////////////////////////////////////////////////////////////////
	// Task 1.3 - Calculate the diffuse term and return that as the result
	///////////////////////////////////////////////////////////////////////////
    vec3 wi = normalize(viewSpaceLightPosition - viewSpacePosition);
    if (dot(n, wi) <= 0) {
        return vec3(0.0);
    }
    
    vec3 diffuse_term = materialColor * (1.0 / PI) * abs(dot(n, wi)) * li;

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
    vec3 metal_term = brdf * materialColor * (dot(n, wi)) * li;
    vec3 microfacet_term = material_metalness * metal_term + (1 - material_metalness) * dielectric_term;

    return material_reflectivity * microfacet_term + (1 - material_reflectivity) * diffuse_term;
    //return brdf * dot(n, wi) * li; //vec3(diffuse_term);
}

vec3 calculateIndirectIllumination(vec3 wo, vec3 n, vec3 materialColor)
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
    
    vec3 diffuse_term = materialColor * (1.0 / PI) * environment_multiplier * texture(irradianceMap, lookup).rgb;

    ///////////////////////////////////////////////////////////////////////////
	// Task 6 - Look up in the reflection map from the perfect specular 
	//          direction and calculate the dielectric and metal terms. 
	/////////////////////////////////////////////////////////////////////////
    
    vec3 wi = normalize(reflect(-wo, n));
    if (has_texture == 1)
    {
        //wi = lightDir;
    }

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
    vec3 metal_term = fresnel * materialColor * li;

    vec3 microfacet_term = material_metalness * metal_term + (1 - material_metalness) * dielectric_term;
    return material_reflectivity * microfacet_term + (1 - material_reflectivity) * diffuse_term;
}

vec3 calculateColorFromTexture()
{
    vec3 texture_color = texture(material_texture, texCoord).xyz;
    // obtain normal from normal map
    vec3 n = texture(normalMap, fs_in.TexCoords).rgb;
    // transform normal vector to range [-1,1]
    n = normalize(n * 2.0 - 1.0);

    vec3 color = texture(material_texture, fs_in.TexCoords).rgb;
    vec3 ambient = 0.1*color;

    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, n), 0.0);
    vec3 diffuse = diff * color;
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, n);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(n, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    return ambient + diffuse + specular;
}

vec3 calculateColor(vec3 wo, vec3 n)
{
    vec3 materialColor = (material_color * has_material_color) + (texture(colorMap, texCoord).xyz * (1 - has_material_color));
	// Direct illumination
	vec3 direct_illumination_term = calculateDirectIllumiunation(wo, n, materialColor);

	// Indirect illumination
	vec3 indirect_illumination_term = calculateIndirectIllumination(wo, n, materialColor);

	///////////////////////////////////////////////////////////////////////////
	// Add emissive term. If emissive texture exists, sample this term.
	///////////////////////////////////////////////////////////////////////////
	vec3 emission_term = material_emission * material_color;
	if (has_emission_texture == 1) {
	    emission_term = texture(emissiveMap, texCoord).xyz;
	}

    return direct_illumination_term + indirect_illumination_term + emission_term;
}

float calculateCoCRadius()
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
    return CoCRadius;
}

void main()
{
	vec3 wo = -normalize(viewSpacePosition);
	vec3 n = normalize(viewSpaceNormal);
    
    fragmentColor = vec4((calculateColorFromTexture() * has_texture) + (calculateColor(wo, n) * (1 - has_texture)), calculateCoCRadius());
}
