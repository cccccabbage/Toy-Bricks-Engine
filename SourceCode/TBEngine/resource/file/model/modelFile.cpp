#include "modelFile.hpp"
#include "TBEngine/utils/log/log.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <string>
#include <filesystem>
#include <algorithm>
#include <unordered_map>

namespace TBE::Resource::File {

using Math::DataFormat::IdxType;
using Math::DataFormat::Vertex;

std::vector<std::string> ModelFile::supportedShaderTypes = {};

ModelFile::ModelFile(std::string_view filePath_) : super(filePath_) {
    if (supportedShaderTypes.empty()) {
        supportedShaderTypes.emplace_back(".obj");
    }
    valid = checkPathValid();
}

void ModelFile::read() {
    if (!vertices.empty()) {
        logger->warn("Model data already exist.");
        return;
    }

    tinyobj::attrib_t                attrib{};
    std::vector<tinyobj::shape_t>    shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string                      warn, err{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.string().c_str())) {
        auto msg = warn + err;
        logger->error(msg);
        throw std::runtime_error(msg);
    }

    std::unordered_map<Vertex, IdxType> uniqueVertices{};

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

bool ModelFile::checkPathValid() {
    bool ret = super::checkPathValid();
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

void ModelFile::releaseOldFile() {
    free();
}

void ModelFile::prepareNewFile() {
}

} // namespace TBE::Resource::File
