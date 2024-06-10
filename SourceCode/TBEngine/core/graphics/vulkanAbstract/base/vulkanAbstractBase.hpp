#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"

namespace TBE::Graphics {

class VulkanAbstractBase {
public:
    VulkanAbstractBase();

public:
    virtual void destroy() = 0;

protected:
    const vk::Device&         device;
    const vk::PhysicalDevice& phyDevice;
};

} // namespace TBE::Graphics
