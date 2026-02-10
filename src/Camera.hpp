#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera();

    void setTarget(const glm::vec3 &target);
    void setDistance(float distance);

    void rotate(float deltaYaw, float deltaPitch);
    void zoom(float delta);

    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;

    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getDistance() const { return distance; }
    glm::vec3 getTarget() const { return target; }

private:
    void updatePosition();

    glm::vec3 target = glm::vec3(0.0f, 80.0f, 0.0f);
    glm::vec3 position = glm::vec3(0.0f, 100.0f, 300.0f);

    float yaw = 0.0f;
    float pitch = 20.0f;
    float distance = 300.0f;

    float minDistance = 10.0f;
    float maxDistance = 1000.0f;
    float minPitch = -89.0f;
    float maxPitch = 89.0f;

    float rotationSpeed = 0.3f;
    float zoomSpeed = 20.0f;
};
