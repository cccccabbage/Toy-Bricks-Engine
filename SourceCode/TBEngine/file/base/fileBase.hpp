#pragma once

#include <string>
#include <filesystem>

namespace TBE::File {

class FileBase {
public:
    FileBase(const std::string& filePath_) : filePath(filePath_), valid(pathValid()) {}

protected:
    const std::filesystem::path filePath{};
    bool                        valid{};

public:
    const std::string getPath() const {
        if (!valid) { return "valid path"; }
        return filePath.string();
    }
    const bool isValid() const { return valid; }

protected:
    virtual bool pathValid() const {
        return (!std::filesystem::is_directory(filePath)) && std::filesystem::exists(filePath);
    }
};

} // namespace TBE::File