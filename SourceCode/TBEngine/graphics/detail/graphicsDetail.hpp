#pragma once

#include "TBEngine/utils/macros/includeVulkan.hpp"
#include "TBEngine/math/dataFormat.hpp"
#include "TBEngine/window/window.hpp"
#include "TBEngine/utils/log/log.hpp"


#include <set>
#include <algorithm>
#include <array>
#include <utility>

#ifdef NDEBUG
#    ifndef TBE_INDEBUG
#        define TBE_INDEBUG
constexpr bool inDebug = false;
#    endif
#else
#    ifndef TBE_INDEBUG
#        define TBE_INDEBUG
constexpr bool inDebug = true;
#    endif
#endif


VKAPI_ATTR vk::Bool32 VKAPI_CALL
           debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                         VkDebugUtilsMessageTypeFlagsEXT             messageType,
                         VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                         void*                                       pUserData);

namespace TBE::Graphics::Detail {
using TBE::Utils::Log::logErrorMsg;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily = std::nullopt;
    std::optional<uint32_t> presentFamily  = std::nullopt;

    QueueFamilyIndices(const vk::PhysicalDevice& device, const vk::SurfaceKHR* surface) {
        auto queueFamilies = device.getQueueFamilyProperties();
        int  i             = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) { graphicsFamily = i; }

            vk::Bool32 value{};
            depackReturnValue(value, device.getSurfaceSupportKHR(i, *surface));
            if (value) { presentFamily = i; }

            if (isComplete()) break;
            i++;
        }
        if (!isComplete()) { logErrorMsg("Cannot find suitable queue families!"); }
    }

    std::set<uint32_t> toSet() const {
        std::set<uint32_t> ret = {};
        if (isComplete()) ret = {graphicsFamily.value(), presentFamily.value()};

        return std::move(ret);
    }

    std::array<uint32_t, 2> toArray() const {
        std::array<uint32_t, 2> ret = {};

        if (isComplete()) ret = {graphicsFamily.value(), presentFamily.value()};

        return std::move(ret);
    }

    bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR        capabilities{};
    std::vector<vk::SurfaceFormatKHR> formats{};
    std::vector<vk::PresentModeKHR>   presentModes{};

    SwapChainSupportDetails(const vk::PhysicalDevice& phyDevice, const vk::SurfaceKHR& surface) {
        depackReturnValue(capabilities, phyDevice.getSurfaceCapabilitiesKHR(surface));
        depackReturnValue(formats, phyDevice.getSurfaceFormatsKHR(surface));
        depackReturnValue(presentModes, phyDevice.getSurfacePresentModesKHR(surface));
    }
};

inline bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers) {
    std::vector<vk::LayerProperties> availableLayers{};
    depackReturnValue(availableLayers, vk::enumerateInstanceLayerProperties());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) return false;
    }

    return true;
}

inline std::vector<const char*> getRequiredExtensions() {
    auto extensions = Window::getRequiredExtensions();

    if (inDebug) extensions.push_back(vk::EXTDebugUtilsExtensionName);

    return extensions;
}

inline vk::DebugUtilsMessengerCreateInfoEXT genDebugCreateInfo() {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo
        .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
        .setPfnUserCallback(debugCallback);

    return std::move(createInfo);
}

inline bool checkDeviceExtensionSupport(const vk::PhysicalDevice&       device,
                                        const std::vector<const char*>& requiredExtensions) {
    std::vector<vk::ExtensionProperties> availableExtensions{};
    depackReturnValue(availableExtensions, device.enumerateDeviceExtensionProperties());

    std::set<std::string> reqExts(requiredExtensions.begin(), requiredExtensions.end());
    auto                  reqSize = reqExts.size();

    for (const auto& extension : availableExtensions) {
        if (reqExts.find(extension.extensionName) != reqExts.end()) reqSize--;
    }

    return reqSize == 0;
}

inline vk::SurfaceFormatKHR
chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availabelFormats) {
    for (const auto& availableFormat : availabelFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    return availabelFormats[0];
}

inline vk::PresentModeKHR
chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) { return availablePresentMode; }
    }

    return vk::PresentModeKHR::eFifo;
}

inline vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR&    capabilities,
                                     const std::pair<uint32_t, uint32_t>& bufferSize) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        vk::Extent2D actualExtent = {bufferSize.first, bufferSize.second};

        actualExtent.width = std::clamp(actualExtent.width,
                                        capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);
        return std::move(actualExtent);
    }
    return {};
}

inline vk::VertexInputBindingDescription getBindingDescription() {
    vk::VertexInputBindingDescription bindDesc{};

    bindDesc.setBinding(0)
        .setStride(sizeof(Math::DataFormat::Vertex))
        .setInputRate(vk::VertexInputRate::eVertex);

    return std::move(bindDesc);
}

inline std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
    std::array<vk::VertexInputAttributeDescription, 3> attrDesc{};

    attrDesc[0]
        .setBinding(0)
        .setLocation(0)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setOffset(offsetof(Math::DataFormat::Vertex, pos));

    attrDesc[1]
        .setBinding(0)
        .setLocation(1)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setOffset(offsetof(Math::DataFormat::Vertex, color));

    attrDesc[2]
        .setBinding(0)
        .setLocation(2)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setOffset(offsetof(Math::DataFormat::Vertex, texCoord));

    return std::move(attrDesc);
}

inline bool hasStencilComponent(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

inline uint32_t findMemoryType(const vk::PhysicalDeviceMemoryProperties& memProperties,
                               uint32_t                                  typeFilter,
                               vk::MemoryPropertyFlags                   properties) {
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    logErrorMsg("failed to find suitable memory type!");
    return {};
}

} // namespace TBE::Graphics::Detail

VKAPI_ATTR inline vk::Bool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT             messageType,
              VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
              void*                                       pUserData) {
    std::string msg = std::string("validation layer: ") + std::string(pCallbackData->pMessage);
    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            msg = std::string("<VERBOSE> ") + msg;
            logger->trace(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            msg = std::string("<INFO> ") + msg;
            logger->info(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            msg = std::string("<WARNING> ") + msg;
            logger->warn(msg);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            msg = std::string("<ERROR> ") + msg;
            logger->error(msg);
            break;
    }
    return vk::False;
}
