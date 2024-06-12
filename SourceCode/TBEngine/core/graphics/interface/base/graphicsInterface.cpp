#include "graphicsInterface.hpp"
#include "TBEngine/core/graphics/graphics.hpp"

namespace TBE::Graphics {

GraphicsInterface::GraphicsInterface()
    : device(Graphics::VulkanGraphics::device), phyDevice(Graphics::VulkanGraphics::phyDevice) {
}

} // namespace TBE::Graphics
