#include "shaderFile.hpp"

#include <string>
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace TBE::Resource::File
{

std::vector<std::string> ShaderFile::supportedShaderTypes = {};

ShaderFile::ShaderFile(const std::string filePath_) : super(filePath_)
{
    if (supportedShaderTypes.empty()) { supportedShaderTypes.emplace_back(".spv"); }
    valid = checkPathValid();
}

std::vector<char> ShaderFile::read() const
{
    std::ifstream file(filePath.string(), std::ios::ate | std::ios::binary);

    if (!file.is_open()) { throw std::runtime_error("failed to open file!"); }

    size_t            fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return std::move(buffer);
}

bool ShaderFile::checkPathValid()
{
    bool ret = super::checkPathValid();
    if (ret)
    {
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

void ShaderFile::releaseOldFile() {}

void ShaderFile::prepareNewFile() {}

} // namespace TBE::Resource::File
