#pragma once

#include "loggers.h"

#include <fmt/format.h>
#include <vulkan/vulkan.h>

#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace im3e {

template <typename E, typename T>
T&& throwIfNull(T&& ptr, std::string_view exceptionMessage)
{
    if (!ptr)
    {
        throw E(exceptionMessage.data());
    }
    return std::forward<T>(ptr);
}

template <typename T>
auto throwIfArgNull(T&& ptr, std::string_view errorMessage) -> T&&
{
    if (!ptr)
    {
        throw std::invalid_argument(errorMessage.data());
    }
    return std::forward<T>(ptr);
}

template <typename E>
void throwIfFalse(bool expression, std::string_view exceptionMessage)
{
    if (!expression)
    {
        throw E(exceptionMessage.data());
    }
}

inline void throwIfVkFailed(VkResult vkResult, std::string_view errorMessage)
{
    throwIfFalse<std::runtime_error>(vkResult == VK_SUCCESS,
                                     fmt::format("Vulkan Error {}: {}", static_cast<int>(vkResult), errorMessage));
}

inline void logIfVkFailed(VkResult vkResult, const ILogger& rLogger, std::string_view errorMessage)
{
    if (vkResult != VK_SUCCESS)
    {
        rLogger.error(fmt::format("Vulkan Error {}: {}", static_cast<int>(vkResult), errorMessage));
    }
}

}  // namespace im3e