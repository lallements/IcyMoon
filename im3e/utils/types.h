#pragma once

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>

namespace im3e {

template <typename T>
using UniquePtrWithDeleter = std::unique_ptr<T, std::function<void(T*)>>;

template <typename T>
using VkUniquePtr = std::unique_ptr<typename std::remove_pointer<T>::type, std::function<void(T)>>;

template <typename T>
using VkSharedPtr = std::shared_ptr<typename std::remove_pointer<T>::type>;

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

}  // namespace im3e