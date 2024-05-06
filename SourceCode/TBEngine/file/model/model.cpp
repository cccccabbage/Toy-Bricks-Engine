#include "TBEngine/file/model/model.hpp"
#include "TBEngine/utils/log/log.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <string>
#include <filesystem>
#include <algorithm>
#include <unordered_map>

namespace TBE::File {

using Math::DataFormat::idxType;
using Math::DataFormat::Vertex;

std::vector<std::string> ModelFile::supportedShaderTypes = {};

ModelFile::ModelFile(const std::string filePath_) : super(filePath_) {
    if (supportedShaderTypes.empty()) { supportedShaderTypes.emplace_back(".obj"); }
    valid = pathValid();
}

void ModelFile::read() {
    if (!vertices.empty()) return;

    tinyobj::attrib_t                attrib{};
    std::vector<tinyobj::shape_t>    shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string                      warn, err{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.string().c_str())) {
        auto msg = warn + err;
        logger->error(msg);
        throw std::runtime_error(msg);
    }

    std::unordered_map<Vertex, idxType> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.pos      = {attrib.vertices[3 * index.vertex_index + 0],
                               attrib.vertices[3 * index.vertex_index + 1],
                               attrib.vertices[3 * index.vertex_index + 2]};
            vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                               1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            vertex.color    = {1.0f, 1.0f, 1.0f};
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void ModelFile::free() {
    vertices.clear();
    indices.clear();
}

bool ModelFile::pathValid() const {
    bool ret = super::pathValid();
    if (ret) {
        const std::string fileExt  = filePath.extension().string();
        bool              contains = false;
        auto              findExt  = [&fileExt, &contains](std::string& supExt) -> void {
            contains = contains || (supExt.compare(fileExt) == 0);
        };
        std::for_each(supportedShaderTypes.begin(), supportedShaderTypes.end(), findExt);
        ret = ret && contains;
    }
    return ret;
}

} // namespace TBE::File