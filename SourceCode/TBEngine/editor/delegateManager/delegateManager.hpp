#pragma once

#include "TBEngine/utils/delegate/delegate.hpp"
#include "TBEngine/utils/log/log.hpp"
#include "TBEngine/utils/macros/includeGLFW.hpp"

#include <any>
#include <unordered_map>

namespace TBE::Editor::DelegateManager {

enum class InputType {
    eUnknown,
    eMouseMove,  // parameters are uint32_t for current x and uint32_t for current y
    eMouseClick, // parameter is a bool, true for right click and false for left click
    eKeyBoard,   // parameter is a KeyType and means down
};

enum class KeyType {
    eNull,
    eEscape,
};

struct DelegateIndexGetter {
    template<typename ... Args>
    static uint32_t get() {
        static uint32_t idx = currentIndex++;
        return idx;
    }

    inline static uint32_t currentIndex = 0;
};

class DelegateManager {
public:
    DelegateManager() {}
    ~DelegateManager() { delegates.clear(); }

public:
    template<typename ... Args>
    void boardcast(uint32_t delegateIndex, Args&&... args) {
        if(!delegates[delegateIndex]) {
            Utils::Log::logErrorMsg("calling for boardcast of a released delegate");
            return;
        }
        auto* p = static_cast<Utils::Delegate<void(Args...)>*>(delegates[delegateIndex].get());
        p->broadcast(std::forward<Args>(args)...);
    }
    
    uint32_t addDelegate(InputType type, std::vector<KeyType> listenKeys = {}) {
        uint32_t ret = 0;
        switch(type) {
            case InputType::eMouseMove:
                ret = addDelegate<uint32_t, uint32_t>();
                break;
            case InputType::eMouseClick:
                ret = addDelegate<bool>();
                break;
            case InputType::eKeyBoard:
                ret = addDelegate<KeyType>();
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

    template<typename... Args>
    void bindFunc(uint32_t delegateIndex, std::any func) {
        if (auto it = delegates.find(delegateIndex); it == delegates.end()) {
            Utils::Log::logErrorMsg("bind func to a delegate with an illegal index");
        }
        auto p = dynamic_cast<Utils::Delegate<void(Args...)>*>(delegates[delegateIndex].get());
        p->bindFunction(func);
    }

private:
    template<typename... Args>
    uint32_t addDelegate() {
        auto idx = DelegateIndexGetter::get<Args...>();
        delegates[idx] = std::make_unique<Utils::Delegate<void(Args...)>>();
        return idx;
    }

    void releaseDelegate(uint32_t delegateIndex) { delegates.erase(delegateIndex); }

private:
    std::unordered_map<uint32_t, std::unique_ptr<Utils::DelegateBase>> delegates {};
};

} // namespace TBE::Editor
