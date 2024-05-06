#include "descriptor.hpp"

namespace TBE::Graphics {

Descriptor::~Descriptor() { destroy(); }

void Descriptor::destroy() {
    if (!pDevice) return;
    if (poolInited) {
        pDevice->destroy(pool);
        poolInited = false;
        setsInited = false;
    }
    if (layoutInited) {
        pDevice->destroy(layout);
        layoutInited = false;
    }
}

void Descriptor::initLayout(const std::span<vk::DescriptorSetLayoutBinding>& bindings) {
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        {}, static_cast<uint32_t>(bindings.size()), bindings.data()};
    depackReturnValue(layout, pDevice->createDescriptorSetLayout(layoutInfo));
    layoutInited = true;
}

void Descriptor::initPool(uint32_t maxSets, const std::span<vk::DescriptorPoolSize> poolSizes) {
    vk::DescriptorPoolCreateInfo poolInfo{{}, maxSets, poolSizes};
    depackReturnValue(pool, pDevice->createDescriptorPool(poolInfo));
    poolInited = true;
}

void Descriptor::initSets(std::span<BufferResourceUniform> uniBuffers,
                          const vk::Sampler&               sampler,
                          const vk::ImageView&             sampleTarget) {
    // the copy here is needed, because one layout is need to be specified for every descriptor set
    auto                                 numSets = static_cast<uint32_t>(uniBuffers.size());
    std::vector<vk::DescriptorSetLayout> layouts(numSets, layout);
    vk::DescriptorSetAllocateInfo        allocInfo{pool, layouts};

    sets.resize(numSets);
    depackReturnValue(sets, pDevice->allocateDescriptorSets(allocInfo));
    for (size_t i = 0; i < numSets; i++) {
        vk::DescriptorBufferInfo bufferInfo{uniBuffers[i].buffer, 0, uniBuffers[i].size};
        vk::DescriptorImageInfo  imageInfo{
            sampler, sampleTarget, vk::ImageLayout::eShaderReadOnlyOptimal};

        std::vector<vk::WriteDescriptorSet> desWrites(2, vk::WriteDescriptorSet{});
        desWrites[0]
            .setDstSet(sets[i])
            .setDstBinding(0)
            .setDstArrayElement(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setPBufferInfo(&bufferInfo);
        desWrites[1]
            .setDstSet(sets[i])
            .setDstBinding(1)
            .setDstArrayElement(0)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDescriptorCount(1)
            .setPImageInfo(&imageInfo);
        pDevice->updateDescriptorSets(desWrites, nullptr);
    }

    setsInited = true;
}

} // namespace TBE::Graphics
