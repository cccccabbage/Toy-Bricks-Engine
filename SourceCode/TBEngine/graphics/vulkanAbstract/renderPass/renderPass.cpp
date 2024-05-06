#include "renderPass.hpp"
#include "TBEngine/graphics/detail/graphicsDetail.hpp"

namespace TBE::Graphics {
using namespace TBE::Graphics::Detail;

RenderPass::~RenderPass() { destroy(); }

void RenderPass::initAll(const vk::Device*       pDevice_,
                         vk::Format              swapchainFormat,
                         vk::Format              depthFormat,
                         vk::SampleCountFlagBits msaaSamples) {
    setPDevice(pDevice_);

    vk::AttachmentDescription colorAttachment{};
    colorAttachment.setFormat(swapchainFormat)
        .setSamples(msaaSamples)
        .setLoadOp(vk::AttachmentLoadOp::eClear)   // what to do before rendering to rgbd data
        .setStoreOp(vk::AttachmentStoreOp::eStore) // what to do after rendering to rgbd data
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)   // applying to stencil data
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare) // applying to stencil data
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(
            vk::ImageLayout::eColorAttachmentOptimal); // msaa-ed image cannot be presented directly

    // the index would be referenced in frag shader   layout(location = 0) out vec4 outColor;
    // other types of attachments are avilable as well.
    // input attachment, resolve attachment, depth stencil attachment, preserve attachment
    vk::AttachmentReference colorAttachmentRef{
        0,                                       // attachment
        vk::ImageLayout::eColorAttachmentOptimal // layout
    };

    vk::AttachmentDescription depthAttachment{};
    depthAttachment.setFormat(depthFormat)
        .setSamples(msaaSamples)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference depthAttachmentRef{
        1,                                              // attachment
        vk::ImageLayout::eDepthStencilAttachmentOptimal // layout
    };

    vk::AttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.setFormat(swapchainFormat)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentResolveRef{2, vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass{};
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&colorAttachmentRef)
        .setPDepthStencilAttachment(&depthAttachmentRef)
        .setPResolveAttachments(&colorAttachmentResolveRef);

    vk::SubpassDependency dependency{};
    dependency.setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                         vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                         vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                          vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    std::array attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size()))
        .setPAttachments(attachments.data())
        .setSubpassCount(1)
        .setPSubpasses(&subpass)
        .setDependencyCount(1)
        .setPDependencies(&dependency);

    depackReturnValue(renderPass, pDevice->createRenderPass(renderPassInfo));
}

void RenderPass::destroy() {
    if (!pDevice) return;
    pDevice->destroy(renderPass);
    pDevice = nullptr;
}


} // namespace TBE::Graphics