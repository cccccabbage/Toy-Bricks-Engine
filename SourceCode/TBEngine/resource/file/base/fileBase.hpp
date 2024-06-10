#pragma once

#include <string_view>
#include <filesystem>

namespace TBE::Resource::File {

class FileBase {
public:
    FileBase(std::string_view filePath_) : filePath(filePath_), valid(checkPathValid()) {}
    FileBase(const std::filesystem::path& filePath_)
        : filePath(filePath_), valid(checkPathValid()) {}

protected:
    std::filesystem::path filePath{};
    bool                  valid{};

public:
    void newFile(std::string_view newPath) { newFile(std::filesystem::path(newPath)); }
    void newFile(const std::filesystem::path newPath) {
        releaseOldFile();
        filePath = newPath;
        if (checkPathValid())
            prepareNewFile();
    }
    const std::string getPath() const {
        if (!valid) {
            return "valid path";
        }
        return filePath.string();
    }
    bool isValid() const { return valid; }

protected:
    virtual bool checkPathValid() {
        valid = (!std::filesystem::is_directory(filePath)) && std::filesystem::exists(filePath);
        return valid;
    }
    virtual void releaseOldFile() = 0;
    virtual void prepareNewFile() = 0;
};

} // namespace TBE::Resource::File
