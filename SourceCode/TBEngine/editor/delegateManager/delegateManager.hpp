#pragma once

#include "TBEngine/utils/delegate/delegate.hpp"
#include "TBEngine/utils/log/log.hpp"

#include <any>
#include <unordered_map>

namespace TBE::Editor::DelegateManager {
using KeyStateMap = uint64_t;

enum class InputType
{
    eUnknown,
    eMouseMove,  // parameters are uint32_t for current x and uint32_t for current y
    eMouseClick, // parameter is a bool, true for right click and false for left click
    eKeyBoard,   // parameter is a KeyStateMap, for each bit of it means a key is down
};

// XXX: this enum class for keys has a limit of 64, needs to be imporved
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

struct DelegateIndexGetter {
    template <typename... Args>
    static uint32_t get() {
        return doGet<std::remove_reference_t<std::remove_cv_t<Args>>...>();
    }

private:
    template <typename... Args>
    inline static uint32_t doGet() {
        static uint32_t idx = currentIndex++;
        return idx;
    }

private:
    inline static uint32_t currentIndex = 0;
};

class DelegateManager {
public:
    DelegateManager() {}
    ~DelegateManager() { delegates.clear(); }

public:
    template <typename... Args>
    void boardcast(Args&&... args) {
        auto delegateIndex = DelegateIndexGetter::get<Args...>();

        if (!delegates[delegateIndex]) {
            Utils::Log::logErrorMsg("calling for boardcast of a released delegate");
            return;
        }
        // auto* p = dynamic_cast<Utils::Delegate<void(Args...)>*>(delegates[delegateIndex].get());
        auto* p = static_cast<Utils::Delegate<void(Args...)>*>(delegates[delegateIndex].get());
        p->boardcast(std::forward<Args>(args)...);
    }

    // TODO: multiple InputType for one function should be fine as well
    // maybe use std::tuple to combine multiple <typename... Args>
    uint32_t addDelegate(InputType type) {
        uint32_t ret = 0;
        switch (type) {
            case InputType::eMouseMove:
                ret = addDelegate<uint32_t, uint32_t>();
                break;
            case InputType::eMouseClick:
                ret = addDelegate<bool>();
                break;
            case InputType::eKeyBoard:
                ret = addDelegate<KeyStateMap>();
                break;
            case InputType::eUnknown:
                Utils::Log::logErrorMsg("bad InputType");
                break;
            default:
                Utils::Log::logErrorMsg("bad InputType");
                break;
        }
        return ret;
    }

    template <typename... Args>
    void bindFunc(uint32_t delegateIndex, std::any func) {
        if (auto it = delegates.find(delegateIndex); it == delegates.end()) {
            Utils::Log::logErrorMsg("bind func to a delegate with an illegal index");
        }
        auto p = dynamic_cast<Utils::Delegate<void(Args...)>*>(delegates[delegateIndex].get());
        p->bindFunction(func);
    }

private:
    template <typename... Args>
    uint32_t addDelegate() {
        auto idx       = DelegateIndexGetter::get<Args...>();
        delegates[idx] = std::make_unique<Utils::Delegate<void(Args...)>>();
        return idx;
    }

    void releaseDelegate(uint32_t delegateIndex) { delegates.erase(delegateIndex); }

private:
    std::unordered_map<uint32_t, std::unique_ptr<Utils::DelegateBase>> delegates{};
};

} // namespace TBE::Editor::DelegateManager
