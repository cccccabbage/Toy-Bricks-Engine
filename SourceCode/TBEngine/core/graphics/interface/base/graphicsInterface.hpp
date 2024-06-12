#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"

namespace TBE::Graphics {

class GraphicsInterface {
public:
    GraphicsInterface();

protected:
    const vk::Device&         device;
    const vk::PhysicalDevice& phyDevice;
};

} // namespace TBE::Graphics
