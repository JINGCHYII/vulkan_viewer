#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inUV;

layout(set = 0, binding = 0) uniform FrameUBO {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} frame;

layout(push_constant) uniform PushConstants {
    mat4 model;
} pushData;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outTangent;
layout(location = 3) out vec2 outUV;

void main() {
    vec4 worldPos = pushData.model * vec4(inPosition, 1.0);
    outWorldPos = worldPos.xyz;
    outNormal = normalize(mat3(pushData.model) * inNormal);
    outTangent = inTangent;
    outUV = inUV;

    gl_Position = frame.proj * frame.view * worldPos;
}
