#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/resource/file/shader/shaderFile.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/descriptor/descriptor.hpp"
#include "TBEngine/enums.hpp"

#include <vector>
#include <string>

namespace TBE::Resource {

class Shader {
public:
    Shader();
    ~Shader();

    void destroy();

    // the cache includes shaderModules, shaderStageInfos and bindings
    void destroyCache();

public:
    void addShader(std::string filePath, ShaderType type);

    // this would init descriptor set layout
    [[nodiscard]] const std::vector<vk::PipelineShaderStageCreateInfo> initDescriptorSetLayout();

public:
    std::vector<vk::ShaderModule>                  modules{};
    std::vector<vk::PipelineShaderStageCreateInfo> stageInfos{};
    std::vector<vk::DescriptorSetLayoutBinding>    bindings{};
    Graphics::Descriptor                           descriptors;

private:
    bool modulesInited    = false;
    bool stageInfosInited = false;
    bool bindingsInited   = false;
    bool descInited       = false;

private:
    const vk::Device&         device;
    const vk::PhysicalDevice& phyDevice;

private:
    [[nodiscard]] vk::ShaderModule               createShaderModule(const std::vector<char>& code);
    [[nodiscard]] vk::DescriptorSetLayoutBinding createBinding(ShaderType type);
};

} // namespace TBE::Resource
