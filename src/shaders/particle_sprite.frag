#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float fragDensity;

layout(location = 0) out vec4 outColor;

void main() {
    // Sphere impostor: discard pixels outside the circle
    vec2 coord = fragTexCoord * 2.0 - 1.0;
    float r2 = dot(coord, coord);

    if (r2 > 1.0) {
        discard;
    }

    // Compute sphere normal for lighting
    float z = sqrt(1.0 - r2);
    vec3 normal = vec3(coord, z);

    // Simple diffuse lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDir), 0.0) * 0.7 + 0.3;

    // Water-like blue color, slightly vary by density
    vec3 baseColor = vec3(0.2, 0.5, 0.9);
    vec3 color = baseColor * diffuse;

    // Slight transparency
    outColor = vec4(color, 0.85);
}
