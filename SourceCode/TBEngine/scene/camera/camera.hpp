#pragma once

#include "TBEngine/utils/macros/includeGLM.hpp"
#include "TBEngine/editor/delegateManager/delegateManager.hpp"

#include <memory>

namespace TBE::Scene {
using TBE::Editor::DelegateManager::KeyBit;
using TBE::Editor::DelegateManager::KeyStateMap;

class Scene;

class Camera {
    friend class Scene;

public:
    Camera();

public:
    void tickCPU();

public:
    void onKeyDown(KeyStateMap keyMap);

private:
    glm::vec3 pos{2.0f, 2.0f, 2.0f};
    glm::vec3 front{-1.0f, -1.0f, -1.0f};
    glm::vec3 up{0.0f, 0.0f, 1.0f};

    float dirty       = true;
    float speed       = 0.05f;
    float sensitivity = 1.0f / 180.0f;

private:
    std::unique_ptr<glm::mat4> view = nullptr;
    std::unique_ptr<glm::mat4> proj = nullptr;

private:
    inline static float minAngleCos = glm::cos(155.0f / 180.0f * glm::pi<float>());

private:
    void setDirty() {
        front = glm::normalize(front);
        up    = glm::normalize(up);
        dirty = true;
    }

    void resetCamera() {
        pos   = {2.0f, 2.0f, 2.0f};
        front = {-1.0f, -1.0f, -1.0f};
        up    = {0.0f, 0.0f, 1.0f};
        setDirty();
    }
};

} // namespace TBE::Scene
