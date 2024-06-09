#pragma once

#include "TBEngine/resource/file/base/fileBase.hpp"

#include <string>
#include <vector>

namespace TBE::Resource::File {

class ShaderFile : public FileBase {
    using super = FileBase;

public:
    ShaderFile(const std::string filePath_);

public:
    std::vector<char> read() const;

private:
    static std::vector<std::string> supportedShaderTypes;

private:
    bool checkPathValid() override;
    void releaseOldFile() override;
    void prepareNewFile() override;
};


} // namespace TBE::Resource::File
