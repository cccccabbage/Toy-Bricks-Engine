#pragma once

#include "TBEngine/file/base/fileBase.hpp"

#include <string>
#include <vector>

namespace TBE::File {

class ShaderFile : public FileBase {
    using super = FileBase;
public:
    ShaderFile(const std::string filePath_);

public:
    std::vector<char> read() const;

private:
    static std::vector<std::string> supportedShaderTypes;

private:
    bool pathValid() const override;
};


} // namespace TBE::File