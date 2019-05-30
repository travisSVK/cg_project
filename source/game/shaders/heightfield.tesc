#version 420

// define the number of CPs in the output patch
layout (vertices = 3) out;

uniform vec3 cameraWorldPos;

// attributes of the input CPs
in vec3 pos[];
in vec2 texCoord[];
//in vec3 normal[];

// attributes of the output CPs
out vec3 pos_tes[];
out vec2 texCoord_tes[];
//out vec3 normal_tes[];

float GetTessLevel(float Distance0, float Distance1)
{
    float AvgDistance = (Distance0 + Distance1) / 2.0f;

    if (AvgDistance <= 20.0f) {
        return 7.0f;
    }
    else if (AvgDistance <= 30.0f) {
        return 5.0f;
    }
    else {
        return 3.0f;
    }
}

void main()
{
    // Set the control points of the output patch
    texCoord_tes[gl_InvocationID] = texCoord[gl_InvocationID];
    //normal_tes[gl_InvocationID] = normal[gl_InvocationID];
    pos_tes[gl_InvocationID] = pos[gl_InvocationID];

    // Calculate the distance from the camera to the three control points
    float CamToVertexDistance0 = distance(cameraWorldPos, pos[0]);
    float CamToVertexDistance1 = distance(cameraWorldPos, pos[1]);
    float CamToVertexDistance2 = distance(cameraWorldPos, pos[2]);

    // Calculate the tessellation levels
    //gl_TessLevelOuter[0] = GetTessLevel(CamToVertexDistance1, CamToVertexDistance0);
    //gl_TessLevelOuter[1] = GetTessLevel(CamToVertexDistance2, CamToVertexDistance1);
    //gl_TessLevelOuter[2] = GetTessLevel(CamToVertexDistance2, CamToVertexDistance0);
    //gl_TessLevelOuter[0] = 3;
    //gl_TessLevelOuter[1] = 3;
    //gl_TessLevelOuter[2] = 3;
    gl_TessLevelOuter[0] = GetTessLevel(CamToVertexDistance1, CamToVertexDistance2);
    gl_TessLevelOuter[1] = GetTessLevel(CamToVertexDistance2, CamToVertexDistance0);
    gl_TessLevelOuter[2] = GetTessLevel(CamToVertexDistance0, CamToVertexDistance1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
}