#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(float fov, glm::vec3 position, glm::vec3 target, float sensitivity = 0.1f);
    ~Camera() = default;

    void Update(float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessKeyboard(int direction, float deltaTime);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;
    glm::vec3 GetPosition() const { return position; }

private:
    void UpdateCameraVectors();

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;

    float mouseSensitivity;
    float fov = -90.0f;

    glm::vec3 target;
};
