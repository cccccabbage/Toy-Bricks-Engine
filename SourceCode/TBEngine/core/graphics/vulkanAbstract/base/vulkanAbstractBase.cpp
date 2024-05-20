#include "vulkanAbstractBase.hpp"
#include "TBEngine/core/graphics/graphics.hpp"

namespace TBE::Graphics
{

VulkanAbstractBase::VulkanAbstractBase()
    : device(VulkanGraphics::device), phyDevice(VulkanGraphics::phyDevice)
{
}

} // namespace TBE::Graphics
