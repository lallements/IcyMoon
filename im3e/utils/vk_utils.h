#pragma once

#include <im3e/utils/throw_utils.h>
#include <im3e/utils/types.h>

#include <fmt/format.h>

#include <string_view>
#include <vector>

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

}  // namespace im3e