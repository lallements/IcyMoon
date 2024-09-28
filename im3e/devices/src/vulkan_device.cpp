#include "vulkan_device.h"

#include <im3e/utils/throw_utils.h>

#include <algorithm>
#include <set>

using namespace im3e;
using namespace std;

namespace {

constexpr float MaxQueuePriority = 1.0F;

auto makeQueueCreateInfos(const VulkanPhysicalDevice& rPhysicalDevice)
{
    set<uint32_t> queueFamilyIndices;
    auto insertToQueueFamilyIndices = [&](const vector<uint32_t>& rIndices) {
        ranges::transform(rIndices, inserter(queueFamilyIndices, queueFamilyIndices.end()), [](auto i) { return i; });
    };
    insertToQueueFamilyIndices(rPhysicalDevice.queueFamilies.computeFamilyIndices);
    insertToQueueFamilyIndices(rPhysicalDevice.queueFamilies.graphicsFamilyIndices);
    insertToQueueFamilyIndices(rPhysicalDevice.queueFamilies.transferFamilyIndices);
    insertToQueueFamilyIndices(rPhysicalDevice.queueFamilies.presentationFamilyIndices);

    vector<VkDeviceQueueCreateInfo> vkQueueCreateInfos;
    vkQueueCreateInfos.reserve(queueFamilyIndices.size());
    for (const auto& queueFamilyIndex : queueFamilyIndices)
    {
        VkDeviceQueueCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        createInfo.queueCount = 1U;
        createInfo.pQueuePriorities = &MaxQueuePriority;
        vkQueueCreateInfos.emplace_back(createInfo);
    }
    return vkQueueCreateInfos;
}

auto makeVk11Features()
{
    return VkPhysicalDeviceVulkan11Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .storageBuffer16BitAccess = VK_TRUE,
    };
}

auto makeVk12Features()
{
    return VkPhysicalDeviceVulkan12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .shaderFloat16 = VK_TRUE,
        .shaderInt8 = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
        .scalarBlockLayout = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
    };
}

auto makeVk13Features()
{
    return VkPhysicalDeviceVulkan13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .synchronization2 = VK_TRUE,
        .maintenance4 = VK_TRUE,
    };
}

auto makeRayTracingPipelineFeatures()
{
    return VkPhysicalDeviceRayTracingPipelineFeaturesKHR{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,  //
        .rayTracingPipeline = VK_TRUE,
    };
}

auto makeAccelerationStructureFeatures()
{
    return VkPhysicalDeviceAccelerationStructureFeaturesKHR{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        .accelerationStructure = VK_TRUE,
    };
}

auto makeRayQueryFeatures()
{
    return VkPhysicalDeviceRayQueryFeaturesKHR{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
        .rayQuery = VK_TRUE,
    };
}

auto createVkDevice(const VulkanInstanceFcts& rInstFcts, const VulkanExtensions& rExtensions,
                    const VulkanPhysicalDevice& rPhysicalDevice)
{
    const auto vkQueueCreateInfos = makeQueueCreateInfos(rPhysicalDevice);
    const auto& rLayers = rExtensions.getLayers();
    const auto& rDeviceExtensions = rExtensions.getDeviceExtensions();

    VkDeviceCreateInfo vkCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(vkQueueCreateInfos.size()),
        .pQueueCreateInfos = vkQueueCreateInfos.data(),
        .enabledLayerCount = static_cast<uint32_t>(rLayers.size()),
        .ppEnabledLayerNames = rLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(rDeviceExtensions.size()),
        .ppEnabledExtensionNames = rDeviceExtensions.data(),
    };

    auto vk11Features = makeVk11Features();
    vkCreateInfo.pNext = &vk11Features;

    auto vk12Features = makeVk12Features();
    vk11Features.pNext = &vk12Features;

    auto vk13Features = makeVk13Features();
    vk12Features.pNext = &vk13Features;

    auto vkRayTracingPipelineFeatures = makeRayTracingPipelineFeatures();
    vk13Features.pNext = &vkRayTracingPipelineFeatures;

    auto vkAccelerationStructureFeatures = makeAccelerationStructureFeatures();
    vkRayTracingPipelineFeatures.pNext = &vkAccelerationStructureFeatures;

    auto vkRayQueryFeatures = makeRayQueryFeatures();
    vkAccelerationStructureFeatures.pNext = &vkRayQueryFeatures;

    VkDevice vkDevice{};
    throwIfVkFailed(rInstFcts.vkCreateDevice(rPhysicalDevice.vkPhysicalDevice, &vkCreateInfo, nullptr, &vkDevice),
                    "Could not create Vulkan logical device");
    return vkDevice;
}

auto createDeviceAndLoadFcts(const VulkanInstance& rInstance, const VulkanPhysicalDevice& rPhysicalDevice,
                             VulkanDeviceFcts& rFcts)
{
    const auto vkDevice = createVkDevice(rInstance.getFcts(), rInstance.getExtensions(), rPhysicalDevice);
    rFcts = rInstance.loadDeviceFcts(vkDevice);
    return VkUniquePtr<VkDevice>(vkDevice, [&rFcts](VkDevice vkDevice) { rFcts.vkDestroyDevice(vkDevice, nullptr); });
}

auto getVkQueues(const VulkanDeviceFcts& rFcts, VkDevice vkDevice, const vector<uint32_t>& rFamilyIndices)
{
    vector<VkQueue> vkQueues;
    for (auto familyIndex : rFamilyIndices)
    {
        VkQueue queue{};
        rFcts.vkGetDeviceQueue(vkDevice, familyIndex, 0U, &queue);
        vkQueues.push_back(queue);
    }
    return vkQueues;
}

}  // namespace

VulkanDevice::VulkanDevice(const ILogger& rLogger, DeviceConfig config)
  : m_pLogger(rLogger.createChild("VulkanDevice"))
  , m_config(move(config))
  , m_instance(*m_pLogger, m_config.isDebugEnabled,
               createVulkanLoader(VulkanLoaderConfig{
                   .isDebugEnabled = config.isDebugEnabled,
               }))
  , m_physicalDevice(m_instance.choosePhysicalDevice(config.isPresentationSupported))
  , m_pVkDevice(createDeviceAndLoadFcts(m_instance, m_physicalDevice, m_fcts))

  , m_vkComputeQueues(getVkQueues(m_fcts, m_pVkDevice.get(), m_physicalDevice.queueFamilies.computeFamilyIndices))
  , m_vkGraphicsQueues(getVkQueues(m_fcts, m_pVkDevice.get(), m_physicalDevice.queueFamilies.graphicsFamilyIndices))
  , m_vkTransferQueues(getVkQueues(m_fcts, m_pVkDevice.get(), m_physicalDevice.queueFamilies.transferFamilyIndices))
  , m_vkPresentationQueues(
        getVkQueues(m_fcts, m_pVkDevice.get(), m_physicalDevice.queueFamilies.presentationFamilyIndices))
{
    m_pLogger->info("Successfully initialized");
}

auto im3e::createDevice(const ILogger& rLogger, DeviceConfig config) -> shared_ptr<IDevice>
{
    return make_shared<VulkanDevice>(rLogger, move(config));
}