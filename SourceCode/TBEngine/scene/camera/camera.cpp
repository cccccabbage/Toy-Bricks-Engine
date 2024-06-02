#include "camera.hpp"
#include "TBEngine/core/graphics/graphics.hpp"
#include "TBEngine/settings.hpp"

#include <array>
#include <tuple>

namespace TBE::Scene {

constexpr std::array directionKeys = {
    std::make_tuple(0,  1.0f, KeyBit::eD),        // x
    std::make_tuple(0, -1.0f, KeyBit::eA),        // x
    std::make_tuple(1,  1.0f, KeyBit::eSpace),    // y
    std::make_tuple(1, -1.0f, KeyBit::eLeftCtrl), // y
    std::make_tuple(2,  1.0f, KeyBit::eW),        // z
    std::make_tuple(2, -1.0f, KeyBit::eS)         // z
};

constexpr std::array rotationKeys = {
    std::make_tuple(0,  1.0f, KeyBit::eUp),    // x
    std::make_tuple(0, -1.0f, KeyBit::eDown),  // x
    std::make_tuple(1,  1.0f, KeyBit::eLeft),  // y
    std::make_tuple(1, -1.0f, KeyBit::eRight), // y
};

constexpr std::array functionKeys = { KeyBit::eR };

Camera::Camera() 
    : view{std::make_unique<glm::mat4>(glm::mat4())}
    , proj{std::make_unique<glm::mat4>(glm::mat4())} {
    front = glm::normalize(front);
    up    = glm::normalize(up);

    auto& extent = Graphics::VulkanGraphics::extent;
    *proj = glm::perspective(
        glm::radians(45.0f),
        (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
        0.1f,
        10.0f);
    (*proj)[1][1] *= -1; // important: Vulkan has a different coordinates from OpenGL
    *view = glm::lookAt(pos, front, up);
}

void Camera::tickCPU() {
    if (dirty) {
        *view = glm::lookAt(pos, pos + front, up);
    }
    dirty = false;
    return;
}

void Camera::onKeyDown(KeyStateMap keyMap) {
    // rotate camera
    glm::vec2 angles = {0.0f, 0.0f}; // rotate by x axis and y axis
    for (const auto& [idx, sign, bit] : rotationKeys) {
        if (keyMap & (KeyStateMap)bit) {
            angles[idx] += sign;
        }
    }
    if (angles[0] != 0 || angles[1] != 0) {
        // the front vector and up vector are always unit vectors 
        // so the dot product stands for cos of their angle
        angles *= sensitivity;

        auto xAxis = glm::normalize(glm::cross(front, up));
        auto yAxis = glm::normalize(up);
        auto trans = glm::mat4(1.0f);
        trans = glm::rotate(trans, angles[0], xAxis);
        trans = glm::rotate(trans, angles[1], yAxis);
        glm::vec3 newFront = glm::normalize(trans * glm::vec4{front, 1.0f});

        if (glm::abs(glm::dot(newFront, up)) > minAngleCos) {
            front = newFront;
            setDirty();
        }
    }

    // move camera
    glm::vec3 direction = {0.0f, 0.0f, 0.0f}; // x, y, z, in camera coordinate
    for (const auto& [idx, sign, bit]: directionKeys) {
        if (keyMap & (KeyStateMap)bit) {
            direction[idx] += sign;
        }
    }
    if (direction.x + direction.y + direction.z != 0) {
        direction = glm::normalize(direction);

        auto deltaPos = direction.x * glm::normalize(glm::cross(front, up)) // x for right
                      + direction.y * glm::normalize(up)                    // y for up
                      + direction.z * glm::normalize(front);                // z for forward
        deltaPos *= speed;
        pos   += deltaPos;

        setDirty();
    }

    if (keyMap & (KeyStateMap)KeyBit::eR) {
        resetCamera();
    }
}

}
