#version 420

layout(triangles, equal_spacing, ccw) in;

uniform mat4 viewProjectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;
uniform mat4 lightMatrix;
layout(binding = 0) uniform sampler2D heightMap;

in vec3 pos_tes[];
in vec2 texCoord_tes[];
//in vec3 normal_tes[];

out vec2 texCoord;
out vec3 viewSpaceNormal;
out vec3 viewSpacePosition;
out vec4 shadowMapCoord;
out float worldZ;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
   	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
   	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
   	// Interpolate the attributes of the output vertex using the barycentric coordinates
   	texCoord = interpolate2D(texCoord_tes[0], texCoord_tes[1], texCoord_tes[2]);
   	//normal_fs = interpolate3D(normal_tes[0], normal_tes[1], normal_tes[2]);
   	vec3 position = interpolate3D(pos_tes[0], pos_tes[1], pos_tes[2]);

    const vec2 size = vec2(2.0,0.0);
    const ivec3 off = ivec3(-1,0,1);

    vec4 height = texture(heightMap, texCoord);
    float s11 = height.x;
    float s01 = textureOffset(heightMap, texCoord, off.xy).x;
    float s21 = textureOffset(heightMap, texCoord, off.zy).x;
    float s10 = textureOffset(heightMap, texCoord, off.yx).x;
    float s12 = textureOffset(heightMap, texCoord, off.yz).x;
    vec3 va = normalize(vec3(size.x, s21-s01, size.y));
    vec3 vb = normalize(vec3(size.y, s12-s10, -size.x));
    vec4 bump = vec4( cross(va,vb), s11 );

   	//gl_Position = viewProjectionMatrix * vec4(pos_fs.x, pos_fs.y * texture(heightMap, texCoord_fs).x, pos_fs.z, 1.0);
    gl_Position = viewProjectionMatrix * vec4(position.x, position.y * bump.a, position.z, 1.0);
    viewSpaceNormal = (normalMatrix * vec4(bump.xyz, 0.0)).xyz;
    viewSpacePosition = (viewMatrix * vec4(position.x, position.y * bump.a, position.z, 1.0)).xyz;
    worldZ = position.z;

    // shadow map coordinate of the position of vert in the light space
    shadowMapCoord = lightMatrix * vec4(viewSpacePosition, 1.0);
    shadowMapCoord.xyz *= vec3(0.5, 0.5, 0.5);
    shadowMapCoord.xyz += shadowMapCoord.w * vec3(0.5, 0.5, 0.5);

    vs_out.FragPos = position;
    vs_out.TexCoords = texCoord;
    vs_out.TangentLightPos = vec3(0);
    vs_out.TangentViewPos = vec3(0);
    vs_out.TangentFragPos = vec3(0);
}