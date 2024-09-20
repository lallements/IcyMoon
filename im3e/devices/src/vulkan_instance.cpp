#include "vulkan_instance.h"

#include <im3e/utils/throw_utils.h>
#include <im3e/vulkan_loaders/vulkan_loaders.h>

using namespace im3e;
using namespace std;

namespace {

auto createVulkanAppInfo()
{
    return VkApplicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "IcyMoonEngine",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),  // NOLINT warning due to Vulkan macro
        .pEngineName = "IcyMoonEngine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),  // NOLINT warning due to Vulkan macro
        .apiVersion = VK_API_VERSION_1_3,           // NOLINT warning due to Vulkan macro
    };
}

auto createVulkanInstanceCreateInfo(const VkApplicationInfo& rVkAppInfo)
{
    return VkInstanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &rVkAppInfo,
    };
}

}  // namespace

VulkanInstance::VulkanInstance()
  : m_pLoader(createVulkanLoader())
  , m_globalFcts(m_pLoader->loadGlobalFcts())
{
    const auto vkAppInfo = createVulkanAppInfo();
    const auto vkCreateInfo = createVulkanInstanceCreateInfo(vkAppInfo);
    VkInstance vkInstance{};
    throwIfVkFailed(m_globalFcts.vkCreateInstance(&vkCreateInfo, nullptr, &vkInstance),
                    "Failed to create Vulkan instance");

    m_fcts = m_pLoader->loadInstanceFcts(vkInstance);
    m_pVkInstance = VkUniquePtr<VkInstance>(vkInstance,
                                            [this](auto vkInst) { m_fcts.vkDestroyInstance(vkInst, nullptr); });
}
