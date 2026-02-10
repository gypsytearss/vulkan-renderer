#pragma once

#include <glm/glm.hpp>

struct Particle
{
    glm::vec4 position; // xyz = position, w = density
    glm::vec4 velocity; // xyz = velocity, w = pressure
};

struct SimulationParams
{
    float deltaTime;
    float smoothingRadius; // h - SPH kernel radius
    float restDensity;     // rho_0
    float gasConstant;     // k - pressure constant
    float viscosity;       // mu
    float gravity;
    uint32_t particleCount;
    float particleRadius;

    // Boundary
    float boundaryMinX;
    float boundaryMaxX;
    float boundaryMinY;
    float boundaryMaxY;
    float boundaryMinZ;
    float boundaryMaxZ;
    float boundaryDamping;
    float padding;
};
