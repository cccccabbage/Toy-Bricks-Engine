#pragma once

#include "TBEngine/utils/includes/includeVulkan.hpp"
#include "TBEngine/enums.hpp"
#include "TBEngine/core/graphics/vulkanAbstract/descriptor/descriptor.hpp"
#include "TBEngine/core/graphics/interface/base/graphicsInterface.hpp"

#include <vector>

namespace TBE::Graphics {

class ShaderInterface final : public GraphicsInterface {
public:
    ShaderInterface();
    ~ShaderInterface();

    void destroy();

    // the cache includes shaderModules, shaderStageInfos and bindings
    void destroyCache();

public:
    void addShader(std::vector<char> shaderCode, ShaderType type);

    // this would init descriptor set layout
    [[nodiscard]] const std::vector<vk::PipelineShaderStageCreateInfo> initDescriptorSetLayout();

public:
    std::vector<vk::ShaderModule>                  modules{};
    std::vector<vk::PipelineShaderStageCreateInfo> stageInfos{};
    std::vector<vk::DescriptorSetLayoutBinding>    bindings{};
    Graphics::Descriptor                           descriptors;

private:
    [[nodiscard]] vk::ShaderModule               createShaderModule(const std::vector<char>& code);
    [[nodiscard]] vk::DescriptorSetLayoutBinding createBinding(ShaderType type);

private:
    using super = GraphicsInterface;
};

} // namespace TBE::Graphics
