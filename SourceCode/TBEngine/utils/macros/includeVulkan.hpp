#pragma once

// This header is only used to include vulkan headers with some specific macros.

#include "TBEngine/utils/log/log.hpp"

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>


#ifdef VULKAN_HPP_NO_EXCEPTIONS

#    define depackReturnValueM(saveValue, expr, logMsg)                                            \
        {                                                                                          \
            auto [result__, value__] = (expr);                                                     \
            if (result__ != vk::Result::eSuccess) { TBE::Utils::Log::logErrorMsg((logMsg)); }      \
            saveValue = std::move(value__);                                                        \
        }

#    define depackReturnValue(saveValue, expr) depackReturnValueM(saveValue, expr, "bad vk::Result")

#    define handleVkResultM(expr, logMsg)                                                          \
        if ((expr) != vk::Result::eSuccess) { TBE::Utils::Log::logErrorMsg((logMsg)); }

#    define handleVkResult(expr) handleVkResultM(expr, "bad vk::Result")

#else

#    define depackReturnValueM(saveValue, expr, logMsg) saveValue = (expr)

#    define depackReturnValue(saveValue, expr) depackReturnValueM(saveValue, expr, "")

#    define handleVkResultM(expr, logMsg) (expr)

#    define handleVkResult(expr) handleVkResultM(expr, "")

#endif
