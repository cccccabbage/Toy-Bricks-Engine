#include "TBEngine/utils/basic/basic.hpp"

#include <filesystem>

namespace TBE::Utils {

void setRootPath() {
    std::string path = std::filesystem::current_path().string(); // get the working dir
    std::replace(path.begin(), path.end(), '\\', '/');
    std::string eraseBehind{"Toy-Bricks-Engine/"}; // target path

    size_t pos = path.find(eraseBehind);
    if (pos != std::string::npos) {
        // remove the words behind the target folder
        path = path.substr(0, pos + eraseBehind.length());
    } else {
        throw std::runtime_error("you need check work path!");
    }
    std::filesystem::current_path(path.c_str()); // set new working dir
}

std::string getTime() {
    auto    t = std::time(nullptr);
    std::tm timeinfo;
    char    buffer[80];
    localtime_s(&timeinfo, &t);
    std::strftime(buffer, sizeof(buffer), "%Y_%m_%d_%H_%M_%S", &timeinfo);
    return std::string(buffer);
}

} // namespace TBE::Utils
