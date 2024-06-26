#pragma once

#include <any>
#include <functional>
#include <unordered_map>

namespace TBE::Utils {

class DelegateBase {
public:
    virtual ~DelegateBase(){}
};

// usage lies at the end of the file
template <typename T=void>
class Delegate;

template <typename ReturnType, typename... Args>
class Delegate<ReturnType(Args...)> : public DelegateBase {
protected:
    using FuncType = std::function<ReturnType(Args...)>;

public:
    Delegate<ReturnType(Args...)>() {}
    virtual ~Delegate<ReturnType(Args...)>() { releaseDelegate(); }

public:
    void removeBindFunc(const size_t key) { funcs.erase(key); }

    const uint32_t bindFunction(FuncType func_) {
        funcs[currentKey] = func_;
        return currentKey++;
    }

    const uint32_t bindFunction(std::any func) {
        auto p = std::any_cast<FuncType> (func);
        return bindFunction(p);
    }

    void boardcast(Args&&... args) {
        for (auto& kv : funcs) {
            kv.second(std::forward<Args>(args)...);
        }
    }

    void releaseDelegate() {
        funcs.clear();
        currentKey = 0;
    }

protected:
    std::unordered_map<size_t, FuncType> funcs;
    uint32_t                             currentKey{0};
};

} // namespace TBE::Utils


// struct T1 // using inherit would be fine as well, just remember to change the t1.delegate.xxxx to
//           // t1.xxxx
// {
//     TBE::Utils::Delegate<void(int, int)> delegate;
// };
//
// struct T2
// {
//     void memFunc1(int, int) {}
//     void memFunc2(int, int, int) {}
// };
//
// inline void func1(int, int) {}
//
// int main()
// {
//     T1   t1{};
//     T2   t2{};
//     auto a1 = t1.delegate.bindFunction(func1);
//     auto a2 = t1.delegate.bindFunction(
//         std::bind(&T2::memFunc1, &t2, std::placeholders::_1, std::placeholders::_2));
//     auto a3 = t1.delegate.bindFunction(
//         std::bind(&T2::memFunc2, &t2, std::placeholders::_1, std::placeholders::_2, 1));
//     t1.delegate.broadcast(1, 1);
//     t1.delegate.removeBindFunc(a1);
//     t1.delegate.broadcast(0, 0);
//     return 0;
// }
