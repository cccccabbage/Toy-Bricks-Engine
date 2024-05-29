#pragma once

#include "TBEngine/utils/macros/includeGLM.hpp"
#include "TBEngine/utils/macros/includeVulkan.hpp"

namespace TBE::Scene
{

class Camera
{
public:
    void tickCPU();

private:
    glm::mat4 view{};
    glm::mat4 proj{};
};

} // namespace TBE::Scene
