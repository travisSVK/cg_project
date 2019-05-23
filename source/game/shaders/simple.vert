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
uniform vec3 cameraPos;
uniform vec3 lightPos;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

void main() 
{
	viewSpaceNormal = (normalMatrix * vec4(normalIn, 0.0)).xyz;
	viewSpacePosition = (modelViewMatrix * vec4(position, 1)).xyz;
	texCoord = texCoordIn;
    //aNormal = normalIn;
    worldZ = vec4(modelMatrix * vec4(position,1)).z;

//    vec3 T = normalize(vec3(modelMatrix * vec4(aTangent, 0.0)));
//    vec3 B = normalize(vec3(modelMatrix * vec4(aBitTangent, 0.0)));
//    vec3 N = normalize(vec3(modelMatrix * vec4(normalIn, 0.0)));
    vs_out.FragPos = vec3(modelMatrix * vec4 (position, 1.0));
    vs_out.TexCoords = texCoord;
    
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vec3 T = normalize(vec3(normalMatrix * aTangent));
    vec3 N = normalize(vec3(normalMatrix * normalIn));

    T = normalize(T - dot(T,N) * N);
    vec3 B = cross(N,T);

    mat3 TBN = transpose(mat3(T,B,N));
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * cameraPos;
    vs_out.TangentFragPos = TBN * vs_out.FragPos;

	gl_Position = modelViewProjectionMatrix * vec4(position,1);
    //worldZ = position.z;
}