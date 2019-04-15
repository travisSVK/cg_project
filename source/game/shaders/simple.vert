#version 420
///////////////////////////////////////////////////////////////////////////////
// Input vertex attributes
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normalIn;
layout(location = 2) in vec2 texCoordIn;
layout(location = 3) in vec3 aNormal;
layout(location = 4) in vec3 aTangent;
layout(location = 5) in vec3 aBitTangent;

///////////////////////////////////////////////////////////////////////////////
// Input uniform variables
///////////////////////////////////////////////////////////////////////////////
uniform mat4 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;

///////////////////////////////////////////////////////////////////////////////
// Output to fragment shader
///////////////////////////////////////////////////////////////////////////////
out vec2 texCoord;
out vec3 viewSpaceNormal;
out vec3 viewSpacePosition;
out float worldZ;

uniform vec3 viewSpaceLightPosition;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

void main() 
{
	gl_Position = modelViewProjectionMatrix * vec4(position,1);
	viewSpaceNormal = (normalMatrix * vec4(normalIn, 0.0)).xyz;
	viewSpacePosition = (modelViewMatrix * vec4(position, 1)).xyz;
	texCoord = texCoordIn;
    //aNormal = normalIn;
    worldZ = vec4(modelMatrix * vec4(position,1)).z;

    vec3 T = normalize(vec3(modelMatrix * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(aBitTangent, 0.0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(normalIn, 0.0)));

    vs_out.FragPos = position;
    vs_out.TexCoords = texCoord;
    mat3 TBN = transpose(mat3(T,B,N));
    vs_out.TangentLightPos = TBN * viewSpaceLightPosition;
    vs_out.TangentViewPos = TBN * viewSpacePosition;
    vs_out.TangentFragPos = TBN * vec3(modelMatrix * vec4(position, 0.0));
    //worldZ = position.z;
}