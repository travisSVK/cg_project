#version 420
///////////////////////////////////////////////////////////////////////////////
// Input vertex attributes
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normalIn;
layout(location = 2) in vec2 texCoordIn;
layout(binding = 0) uniform sampler2D heightMap;

///////////////////////////////////////////////////////////////////////////////
// Input uniform variables
///////////////////////////////////////////////////////////////////////////////

uniform mat4 modelMatrix;
//uniform mat4 normalMatrix;
//uniform mat4 modelViewMatrix;
//uniform mat4 modelViewProjectionMatrix;

///////////////////////////////////////////////////////////////////////////////
// Output to tcs
///////////////////////////////////////////////////////////////////////////////
out vec2 texCoord;
out vec3 pos;
//out vec3 normal;

void main() 
{
	//gl_Position = modelViewProjectionMatrix * vec4(position.x, texture(heightMap, texCoordIn).x, position.z, 1.0);
//    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
//    gl_Position.y = texture(heightMap, texCoordIn).x;
    //viewSpaceNormal = (normalMatrix * vec4(normalIn, 0.0)).xyz;
    pos = (modelMatrix * vec4(position, 1.0)).xyz;
	texCoord = texCoordIn;
    //normal = normalIn;
}
