#pragma once

#include "TBEngine/utils/includes/includeGLM.hpp"

namespace TBE::Math::DataFormat {

using IdxType = uint32_t;

struct Vertex {
    glm::vec3 pos{};
    glm::vec3 color{};
    glm::vec2 texCoord{};

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model{};
    alignas(16) glm::mat4 view{};
    alignas(16) glm::mat4 proj{};
};

} // namespace TBE::Math::DataFormat

namespace std {
template <>
struct hash<TBE::Math::DataFormat::Vertex> {
    size_t operator()(TBE::Math::DataFormat::Vertex const& vertex) const {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};
} // namespace std
