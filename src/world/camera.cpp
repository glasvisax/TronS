#include "camera.h"
#include <gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>

constexpr glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

Camera::Camera(float fov, glm::vec3 position, glm::vec3 target, float sensitivity) :
    position(position),  
    target(target),
    mouseSensitivity(0.1f),
    fov(fov)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position, target, worldUp);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
}

void Camera::UpdateCameraVectors() {
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
