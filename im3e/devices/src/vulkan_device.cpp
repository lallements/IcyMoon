#include "devices.h"

#include "vulkan_command_buffers.h"
#include "vulkan_images.h"
#include "vulkan_instance.h"
#include "vulkan_memory_allocator.h"

#include <im3e/api/device.h>
#include <im3e/api/image.h>
#include <im3e/api/logger.h>
#include <im3e/api/vulkan_loader.h>
#include <im3e/utils/throw_utils.h>
#include <im3e/utils/types.h>
#include <im3e/utils/vk_utils.h>

#include <algorithm>
#include <memory>
#include <set>
#include <vector>

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

auto findQueue(const VulkanDeviceFcts& rFcts, VkDevice vkDevice, const vector<uint32_t>& rFamilyIndices)
    -> VulkanCommandQueueInfo
{
    for (auto familyIndex : rFamilyIndices)
    {
        VkQueue vkQueue{};
        rFcts.vkGetDeviceQueue(vkDevice, familyIndex, 0U, &vkQueue);
        return VulkanCommandQueueInfo{
            .vkQueue = vkQueue,
            .queueFamilyIndex = familyIndex,
        };
    }
    return VulkanCommandQueueInfo{};
}

auto findCommandQueueInfo(const VulkanDeviceFcts& rFcts, VkDevice vkDevice,
                          const VulkanPhysicalDevice::QueueFamilies& rQueueFamilies)
{
    if (auto queueInfo = findQueue(rFcts, vkDevice, rQueueFamilies.presentationFamilyIndices); queueInfo.vkQueue)
    {
        return queueInfo;
    }
    if (auto queueInfo = findQueue(rFcts, vkDevice, rQueueFamilies.graphicsFamilyIndices); queueInfo.vkQueue)
    {
        return queueInfo;
    }
    throw runtime_error("Could not find Vulkan command queue with either presentation or graphics capabilities");
}

class VulkanDevice : public IDevice, public enable_shared_from_this<VulkanDevice>
{
public:
    VulkanDevice(const ILogger& rLogger, DeviceConfig config)
      : m_pLogger(rLogger.createChild("VulkanDevice"))
      , m_config(move(config))

      , m_instance(*m_pLogger, m_config.isDebugEnabled,
                   createVulkanLoader(VulkanLoaderConfig{
                       .isDebugEnabled = config.isDebugEnabled,
                   }))
      , m_physicalDevice(m_instance.choosePhysicalDevice(config.isPresentationSupported))
      , m_pVkDevice(createDeviceAndLoadFcts(m_instance, m_physicalDevice, m_fcts))
      , m_commandQueueInfo(findCommandQueueInfo(m_fcts, m_pVkDevice.get(), m_physicalDevice.queueFamilies))
    {
        m_pLogger->info("Successfully initialized");
    }

    auto createLogger(std::string_view name) const -> std::unique_ptr<ILogger> override
    {
        return m_pLogger->createChild(name);
    }

    auto getVkInstance() const -> VkInstance override { return m_instance.getVkInstance(); }
    auto getVkPhysicalDevice() const -> VkPhysicalDevice override { return m_physicalDevice.vkPhysicalDevice; }
    auto getVkDevice() const -> VkDevice override { return m_pVkDevice.get(); }
    auto getFcts() const -> const VulkanDeviceFcts& override { return m_fcts; }
    auto getInstanceFcts() const -> const VulkanInstanceFcts& override { return m_instance.getFcts(); }
    auto getMemoryAllocator() const -> shared_ptr<IMemoryAllocator> override
    {
        if (!m_pMemoryAllocator)
        {
            m_pMemoryAllocator = createVulkanMemoryAllocator(shared_from_this(),
                                                             m_instance.loadVmaFcts(m_pVkDevice.get()));
        }
        return m_pMemoryAllocator;
    }
    auto getImageFactory() const -> shared_ptr<const IImageFactory> override
    {
        if (!m_pImageFactory)
        {
            m_pImageFactory = createVulkanImageFactory(shared_from_this());
        }
        return m_pImageFactory;
    }
    auto getCommandQueue() const -> shared_ptr<const ICommandQueue> override
    {
        if (!m_pCommandQueue)
        {
            m_pCommandQueue = createVulkanCommandQueue(shared_from_this(), m_commandQueueInfo, "MainQueue");
        }
        return m_pCommandQueue;
    }
    auto getCommandQueue() -> shared_ptr<ICommandQueue> override
    {
        if (!m_pCommandQueue)
        {
            m_pCommandQueue = createVulkanCommandQueue(shared_from_this(), m_commandQueueInfo, "MainQueue");
        }
        return m_pCommandQueue;
    }

private:
    unique_ptr<ILogger> m_pLogger;
    const DeviceConfig m_config;

    const VulkanInstance m_instance;
    const VulkanPhysicalDevice m_physicalDevice;
    VulkanDeviceFcts m_fcts;
    VkUniquePtr<VkDevice> m_pVkDevice;
    const VulkanCommandQueueInfo m_commandQueueInfo;

    mutable shared_ptr<IMemoryAllocator> m_pMemoryAllocator;
    mutable shared_ptr<IImageFactory> m_pImageFactory;
    mutable shared_ptr<ICommandQueue> m_pCommandQueue;
};

}  // namespace

auto im3e::createDevice(const ILogger& rLogger, DeviceConfig config) -> shared_ptr<IDevice>
{
    return make_shared<VulkanDevice>(rLogger, move(config));
}