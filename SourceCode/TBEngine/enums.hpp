#pragma once
#include <cinttypes>
#include <string>

namespace TBE {

enum class ShaderType
{
    eUnknown = 0,
    eVertex,
    eFrag,
};

enum class InputType
{
    eUnknown,
    eMouseMove,  // parameters are uint32_t for current x and uint32_t for current y
    eMouseClick, // parameter is a bool, true for right click and false for left click
    eKeyBoard,   // parameter is a KeyStateMap, for each bit of it means a key is down
};

// XXX: this enum class for keys has a limit of 64 keys, needs to be imporved
enum class KeyBit : uint64_t
{
    eNull      = 0x0000000000000000,
    eEscape    = 0x0000000000000001,
    eW         = eEscape << 1,
    eA         = eW << 1,
    eS         = eA << 1,
    eD         = eS << 1,
    eLeftCtrl  = eD << 1,
    eSpace     = eLeftCtrl << 1,
    eLeftShift = eSpace << 1,
    eLeft      = eLeftShift << 1,
    eRight     = eLeft << 1,
    eUp        = eRight << 1,
    eDown      = eUp << 1,
    eR         = eDown << 1,
};

enum class ImageResourceType
{
    eUnknown = 0,
    eColor,
    eDepth,
    eTexture,
};

constexpr inline std::string toStringView(ShaderType type) {
    std::string ret = nullptr;
    switch (type) {
        case ShaderType::eUnknown:
            ret = "ShaderType::eUnknown";
            break;
        case ShaderType::eVertex:
            ret = "ShaderType::eVertex";
            break;
        case ShaderType::eFrag:
            ret = "ShaderType::eFrag";
            break;
    }
    return ret;
}

} // namespace TBE
