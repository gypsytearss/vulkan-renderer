#include "Camera.hpp"

#include <algorithm>
#include <cmath>

Camera::Camera()
{
    updatePosition();
}

void Camera::setTarget(const glm::vec3 &newTarget)
{
    target = newTarget;
    updatePosition();
}

void Camera::setDistance(float newDistance)
{
    distance = std::clamp(newDistance, minDistance, maxDistance);
    updatePosition();
}

void Camera::rotate(float deltaYaw, float deltaPitch)
{
    yaw += deltaYaw * rotationSpeed;
    pitch += deltaPitch * rotationSpeed;

    pitch = std::clamp(pitch, minPitch, maxPitch);

    // Keep yaw in [0, 360) range
    while (yaw < 0.0f)
        yaw += 360.0f;
    while (yaw >= 360.0f)
        yaw -= 360.0f;

    updatePosition();
}

void Camera::zoom(float delta)
{
    distance -= delta * zoomSpeed;
    distance = std::clamp(distance, minDistance, maxDistance);
    updatePosition();
}

void Camera::updatePosition()
{
    float yawRad = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);

    position.x = target.x + distance * cos(pitchRad) * sin(yawRad);
    position.y = target.y + distance * sin(pitchRad);
    position.z = target.z + distance * cos(pitchRad) * cos(yawRad);
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 Camera::getPosition() const
{
    return position;
}
