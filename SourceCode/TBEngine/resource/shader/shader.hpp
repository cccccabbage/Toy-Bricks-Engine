#pragma once

#include "TBEngine/resource/file/shader/shaderFile.hpp"
#include "TBEngine/enums.hpp"

#include <vector>
#include <string>

namespace TBE::Resource {

class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();

    void destroy();

public:
    void addShader(std::string filePath, ShaderType type);

private:
    std::vector<std::pair<File::ShaderFile, ShaderType>> shaderFiles{};
};

} // namespace TBE::Resource
