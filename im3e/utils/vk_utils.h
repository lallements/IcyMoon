#pragma once

#include <im3e/utils/throw_utils.h>
#include <im3e/utils/types.h>

#include <fmt/format.h>
#include <vk_mem_alloc.h>

#include <string_view>
#include <vector>

constexpr bool operator==(const VkExtent2D& rVkExtent1, const VkExtent2D& rVkExtent2)
{
    return rVkExtent1.width == rVkExtent2.width && rVkExtent1.height == rVkExtent2.height;
}

constexpr bool operator!=(const VkExtent2D& rVkExtent1, const VkExtent2D& rVkExtent2)
{
    return rVkExtent1.width != rVkExtent2.width || rVkExtent1.height != rVkExtent2.height;
}

namespace im3e {

template <typename T, typename F, typename... Args>
std::vector<T> getVkList(F fct, std::string_view name, Args... args)
{
    uint32_t count{};
    throwIfVkFailed(fct(args..., &count, nullptr), fmt::format("Could not get count for {}", name));

    std::vector<T> result(count);
    throwIfVkFailed(fct(args..., &count, result.data()), fmt::format("Could not get list for {}", name));
    return result;
}

template <class T>
constexpr VkFlags toVkFlags(T vkFlag)
{
    return static_cast<VkFlags>(vkFlag);
}

template <class T, class... Args>
constexpr VkFlags toVkFlags(T vkFlag, Args... args)
{
    return static_cast<VkFlags>(vkFlag) | toVkFlags(args...);
}

template <class T>
constexpr bool vkFlagsContain(VkFlags vkFlags, T vkFlag)
{
    return (vkFlags & static_cast<VkFlags>(vkFlag)) != 0U;
}

}  // namespace im3e