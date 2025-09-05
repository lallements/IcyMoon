#pragma once

#include <functional>
#include <memory>
#include <typeindex>

namespace im3e {

template <typename T>
auto createTypeIndex()
{
    return std::type_index(typeid(T));
}

template <typename T>
using UniquePtrWithDeleter = std::unique_ptr<T, std::function<void(T*)>>;

// clang-format off

// Helper to detect less than operator
template <typename T, typename = void>
struct HasLessThanOperator : std::false_type {};
template <typename T>
struct HasLessThanOperator<T, std::void_t<decltype(std::declval<T>() < std::declval<T>())>> : std::true_type {};

// clang-format on

}  // namespace im3e