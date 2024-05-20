#include "shader.hpp"
#include "TBEngine/core/graphics/graphics.hpp"

namespace TBE::Resource
{
using namespace TBE::Utils::Log;
using File::ShaderFile;

Shader::Shader()
    : device(Graphics::VulkanGraphics::device)
    , phyDevice(Graphics::VulkanGraphics::phyDevice)
    , descriptors()
{
}

Shader::~Shader()
{
    destroy();
}

void Shader::destroy()
{
    destroyCache();
    if (descInited)
    {
        descriptors.destroy();
        descInited = false;
    }
}

void Shader::destroyCache()
{
    if (modulesInited)
    {
        if (!modules.empty())
        {
            for (auto& module : modules)
            {
                device.destroy(module);
            }
            modules.clear();
        }
        modulesInited = false;
    }
    if (stageInfosInited)
    {
        stageInfos.clear();
        stageInfosInited = false;
    }
    if (bindingsInited)
    {
        bindings.clear();
        bindingsInited = false;
    }
}

void Shader::addShader(std::string filePath, ShaderType type)
{
    ShaderFile shaderFile{filePath};

    auto shaderCode = shaderFile.read();
    modules.emplace_back(createShaderModule(shaderCode));
    modulesInited = true;

    vk::PipelineShaderStageCreateInfo shaderStageInfo{{}, {}, modules.back(), "main"};
    switch (type)
    {
        case ShaderType::eVertex: shaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex); break;
        case ShaderType::eFrag: shaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment); break;
        case ShaderType::eUnknown:
        default: logErrorMsg("illegal ShaderType"); break;
    }
    stageInfos.emplace_back(shaderStageInfo);
    stageInfosInited = true;

    bindings.emplace_back(createBinding(type));
    bindingsInited = true;
}

vk::ShaderModule Shader::createShaderModule(const std::vector<char>& code)
{
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.setCodeSize(code.size()).setPCode(reinterpret_cast<const uint32_t*>(code.data()));

    vk::ShaderModule shaderModule{};
    depackReturnValue(shaderModule, device.createShaderModule(createInfo));

    return std::move(shaderModule);
}

const std::vector<vk::PipelineShaderStageCreateInfo> Shader::initDescriptorSetLayout()
{
    if (stageInfos.empty() || bindings.empty())
    {
        logger->warn("call for empty shaderStageInfos or bindings");
    }
    if (!bindings.empty())
    {
        descriptors.initLayout(bindings);
        descInited = true;
    }
    return stageInfos;
}

vk::DescriptorSetLayoutBinding Shader::createBinding(ShaderType type)
{
    vk::DescriptorSetLayoutBinding binding{};
    switch (type)
    {
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
        default: logErrorMsg("illegal ShaderType"); break;
    }
    return std::move(binding);
}

} // namespace TBE::Resource
