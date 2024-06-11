#include "shaderInterface.hpp"
#include "TBEngine/core/graphics/graphics.hpp"

namespace TBE::Graphics {
using namespace TBE::Utils::Log;

ShaderInterface::ShaderInterface()
    : device(Graphics::VulkanGraphics::device)
    , phyDevice(Graphics::VulkanGraphics::phyDevice)
    , descriptors() {
}

ShaderInterface::~ShaderInterface() {
    destroy();
}

void ShaderInterface::destroy() {
    static bool destroyed = false;
    if (!destroyed) {
        destroyCache();
        descriptors.destroy();
        destroyed = true;
    }
}

void ShaderInterface::destroyCache() {
    static bool destroyed = false;
    if (!destroyed) {
        if (!modules.empty()) {
            for (auto& module : modules) {
                device.destroy(module);
            }
            modules.clear();
        }
        stageInfos.clear();
        bindings.clear();
        destroyed = true;
    }
}

void ShaderInterface::addShader(std::vector<char> shaderCode, ShaderType type) {
    modules.emplace_back(createShaderModule(shaderCode));

    vk::PipelineShaderStageCreateInfo shaderStageInfo{{}, {}, modules.back(), "main"};
    switch (type) {
        case ShaderType::eVertex:
            shaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex);
            break;
        case ShaderType::eFrag:
            shaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment);
            break;
        case ShaderType::eUnknown:
        default:
            logErrorMsg("illegal ShaderType");
            break;
    }
    stageInfos.emplace_back(shaderStageInfo);

    bindings.emplace_back(createBinding(type));
}

vk::ShaderModule ShaderInterface::createShaderModule(const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.setCodeSize(code.size()).setPCode(reinterpret_cast<const uint32_t*>(code.data()));

    vk::ShaderModule shaderModule{};
    depackReturnValue(shaderModule, device.createShaderModule(createInfo));

    return std::move(shaderModule);
}

const std::vector<vk::PipelineShaderStageCreateInfo> ShaderInterface::initDescriptorSetLayout() {
    if (stageInfos.empty() || bindings.empty()) {
        logger->warn("call for empty shaderStageInfos or bindings");
    }
    if (!bindings.empty()) {
        descriptors.initLayout(bindings);
    }
    return stageInfos;
}

vk::DescriptorSetLayoutBinding ShaderInterface::createBinding(ShaderType type) {
    vk::DescriptorSetLayoutBinding binding{};
    switch (type) {
        case ShaderType::eVertex:
            binding.setBinding(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
                .setStageFlags(vk::ShaderStageFlagBits::eVertex);
            break;
        case ShaderType::eFrag:
            binding.setBinding(1)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setPImmutableSamplers(nullptr)
                .setStageFlags(vk::ShaderStageFlagBits::eFragment);
            break;
        case ShaderType::eUnknown:
        default:
            logErrorMsg("illegal ShaderType");
            break;
    }
    return std::move(binding);
}

} // namespace TBE::Graphics
