#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"

namespace TBE::Graphics {

class VulkanAbstractBase {
public:
    virtual void destroy() = 0;

public:
    void setPDevice(const vk::Device* pDevice_) { pDevice = pDevice_; }

protected:
    const vk::Device* pDevice = nullptr;
};

} // namespace TBE::Graphics