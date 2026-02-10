#version 450

// Per-instance particle data
layout(location = 0) in vec4 inPosition;  // xyz = position, w = density
layout(location = 1) in vec4 inVelocity;  // xyz = velocity, w = pressure

// Per-vertex quad data
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out float fragDensity;

layout(push_constant) uniform PushConstants {
    mat4 view;
    mat4 proj;
    float particleRadius;
    float padding1;
    float padding2;
    float padding3;
};

// Quad vertices for billboard
const vec2 quadVertices[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0,  1.0),
    vec2(-1.0,  1.0)
);

void main() {
    vec2 quadVertex = quadVertices[gl_VertexIndex];

    // Transform particle position to view space
    vec4 viewPos = view * vec4(inPosition.xyz, 1.0);

    // Billboard: offset in view space
    viewPos.xy += quadVertex * particleRadius;

    // Project to clip space
    gl_Position = proj * viewPos;

    // Pass to fragment shader
    fragTexCoord = quadVertex * 0.5 + 0.5;
    fragDensity = inPosition.w;
}
