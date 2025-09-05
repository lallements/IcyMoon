#pragma once

#include <vulkan/vulkan.h>

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

template <typename T>
using VkUniquePtr = std::unique_ptr<typename std::remove_pointer<T>::type, std::function<void(T)>>;

template <typename T>
using VkSharedPtr = std::shared_ptr<typename std::remove_pointer<T>::type>;

template <typename T>
using VkWeakPtr = std::weak_ptr<typename std::remove_pointer<T>::type>;

template <typename T>
auto makeVkUniquePtr(VkDevice vkDevice, T pValue,
                     std::function<void(VkDevice, T, const VkAllocationCallbacks*)> destructor)
{
    return VkUniquePtr<T>(pValue, [vkDevice, destructor](auto* pV) {
        if (destructor)
        {
            destructor(vkDevice, pV, nullptr);
        }
    });
}

// clang-format off

// Helper to detect less than operator
template <typename T, typename = void>
struct HasLessThanOperator : std::false_type {};
template <typename T>
struct HasLessThanOperator<T, std::void_t<decltype(std::declval<T>() < std::declval<T>())>> : std::true_type {};

// clang-format on

}  // namespace im3e