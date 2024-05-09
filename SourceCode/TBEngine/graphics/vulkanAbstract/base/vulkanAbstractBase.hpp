#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"

namespace TBE::Graphics {

class VulkanAbstractBase {
public:
    VulkanAbstractBase() = delete;
    VulkanAbstractBase(const vk::Device& device_, const vk::PhysicalDevice& phyDevice_)
        : device(device_), phyDevice(phyDevice_) {}

public:
    virtual void destroy() = 0;

protected:
    const vk::Device&         device;
    const vk::PhysicalDevice& phyDevice;
};

} // namespace TBE::Graphics
