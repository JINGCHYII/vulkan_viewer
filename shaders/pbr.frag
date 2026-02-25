#version 460

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform FrameUBO {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} frame;

layout(set = 1, binding = 0) uniform MaterialUBO {
    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
} material;

layout(set = 1, binding = 1) uniform sampler2D baseColorMap;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessMap;
layout(set = 1, binding = 3) uniform sampler2D normalMap;

const float PI = 3.14159265358979323846;

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / max(denom, 1e-4);
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec4 baseColor = texture(baseColorMap, inUV) * material.baseColorFactor;
    vec2 mr = texture(metallicRoughnessMap, inUV).bg;
    float metallic = clamp(mr.x * material.metallicFactor, 0.0, 1.0);
    float roughness = clamp(mr.y * material.roughnessFactor, 0.04, 1.0);

    vec3 N = normalize(inNormal);
    vec3 V = normalize(frame.cameraPos - inWorldPos);
    vec3 L = normalize(vec3(0.4, 0.8, 0.2));
    vec3 H = normalize(V + L);

    vec3 F0 = mix(vec3(0.04), baseColor.rgb, metallic);

    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 1e-4;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = vec3(4.0);
    vec3 Lo = (kD * baseColor.rgb / PI + specular) * radiance * NdotL;

    vec3 ambient = baseColor.rgb * 0.03;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    outColor = vec4(color, baseColor.a);
}
