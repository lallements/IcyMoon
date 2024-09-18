#pragma once

#include <stdexcept>
#include <string>
#include <type_traits>

namespace im3e {

template <typename E, typename T>
T&& throwIfNull(T&& ptr, const std::string& exceptionMessage)
{
    if (!ptr)
    {
        throw E(exceptionMessage);
    }
    return std::forward<T>(ptr);
}

template <typename T>
auto throwIfArgNull(T&& ptr, const std::string& rErrorMessage) -> T&&
{
    if (!ptr)
    {
        throw std::invalid_argument(rErrorMessage);
    }
    return std::forward<T>(ptr);
}

template <typename E>
void throwIfFalse(bool expression, const std::string& exceptionMessage)
{
    if (!expression)
    {
        throw E(exceptionMessage);
    }
}

}  // namespace im3e