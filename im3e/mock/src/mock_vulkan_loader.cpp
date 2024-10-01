#include "mock_vulkan_loader.h"

#include <im3e/test_utils/test_utils.h>
#include <im3e/utils/throw_utils.h>

using namespace im3e;
using namespace std;

namespace {

MockVulkanLoader* g_pMock{};

void assertMockExists()
{
    throwIfNull<logic_error>(g_pMock, "An instance of MockVulkanLoader is required");
}

class MockProxyVulkanLoader : public IVulkanLoader
{
public:
    MockProxyVulkanLoader(MockVulkanLoader& rMock)
      : m_rMock(rMock)
    {
    }

    auto loadGlobalFcts() const -> VulkanGlobalFcts override { return m_rMock.loadGlobalFcts(); }
    auto loadInstanceFcts(VkInstance vkInstance) const -> VulkanInstanceFcts override
    {
        return m_rMock.loadInstanceFcts(vkInstance);
    }
    auto loadDeviceFcts(VkDevice vkDevice) const -> VulkanDeviceFcts override
    {
        return m_rMock.loadDeviceFcts(vkDevice);
    }
    auto loadVmaFcts(VkInstance vkInstance, VkDevice vkDevice) const -> VmaVulkanFunctions override
    {
        return m_rMock.loadVmaFcts(vkInstance, vkDevice);
    }

private:
    MockVulkanLoader& m_rMock;
};

VkResult vkEnumerateInstanceVersion(uint32_t* pApiVersion)
{
    assertMockExists();
    return g_pMock->getMockGlobalFcts().vkEnumerateInstanceVersion(pApiVersion);
}

VkResult vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                VkExtensionProperties* pProperties)
{
    assertMockExists();
    return g_pMock->getMockGlobalFcts().vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}

VkResult vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties)
{
    assertMockExists();
    return g_pMock->getMockGlobalFcts().vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

VkResult vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                          VkInstance* pInstance)
{
    assertMockExists();
    return g_pMock->getMockGlobalFcts().vkCreateInstance(pCreateInfo, pAllocator, pInstance);
}

void vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
    assertMockExists();
    g_pMock->getMockInstanceFcts().vkDestroyInstance(instance, pAllocator);
}

VkResult vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                        const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    assertMockExists();
    return g_pMock->getMockInstanceFcts().vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
    assertMockExists();
    return g_pMock->getMockInstanceFcts().vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                     const VkAllocationCallbacks* pAllocator)
{
    assertMockExists();
    g_pMock->getMockInstanceFcts().vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

VkResult vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                    VkPhysicalDevice* pPhysicalDevices)
{
    assertMockExists();
    return g_pMock->getMockInstanceFcts().vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

void vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties)
{
    assertMockExists();
    g_pMock->getMockInstanceFcts().vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
}

void vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures)
{
    assertMockExists();
    g_pMock->getMockInstanceFcts().vkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

VkResult vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                              uint32_t* pPropertyCount, VkExtensionProperties* pProperties)
{
    assertMockExists();
    return g_pMock->getMockInstanceFcts().vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName,
                                                                               pPropertyCount, pProperties);
}

void vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                              VkQueueFamilyProperties* pQueueFamilyProperties)
{
    assertMockExists();
    g_pMock->getMockInstanceFcts().vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                            pQueueFamilyProperties);
}

void vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                         VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
    assertMockExists();
    g_pMock->getMockInstanceFcts().vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

void vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    assertMockExists();
    g_pMock->getMockDeviceFcts().vkDestroyDevice(device, pAllocator);
}

void vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
{
    assertMockExists();
    g_pMock->getMockDeviceFcts().vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

}  // namespace

MockGlobalFcts::MockGlobalFcts() = default;
MockGlobalFcts::~MockGlobalFcts() = default;

MockInstanceFcts::MockInstanceFcts() = default;
MockInstanceFcts::~MockInstanceFcts() = default;

MockDeviceFcts::MockDeviceFcts() = default;
MockDeviceFcts::~MockDeviceFcts() = default;

MockVulkanLoader::MockVulkanLoader()
  : m_gFcts(VulkanGlobalFcts{
        .vkEnumerateInstanceVersion = &vkEnumerateInstanceVersion,
        .vkEnumerateInstanceExtensionProperties = &vkEnumerateInstanceExtensionProperties,
        .vkEnumerateInstanceLayerProperties = &vkEnumerateInstanceLayerProperties,
        .vkCreateInstance = vkCreateInstance,
    })
  , m_iFcts(VulkanInstanceFcts{
        .vkDestroyInstance = vkDestroyInstance,
        .vkCreateDevice = vkCreateDevice,

        .vkCreateDebugUtilsMessengerEXT = vkCreateDebugUtilsMessengerEXT,
        .vkDestroyDebugUtilsMessengerEXT = vkDestroyDebugUtilsMessengerEXT,

        .vkEnumeratePhysicalDevices = vkEnumeratePhysicalDevices,
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceFeatures = vkGetPhysicalDeviceFeatures,
        .vkEnumerateDeviceExtensionProperties = vkEnumerateDeviceExtensionProperties,
        .vkGetPhysicalDeviceQueueFamilyProperties = vkGetPhysicalDeviceQueueFamilyProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
    })
  , m_dFcts(VulkanDeviceFcts{
        .vkDestroyDevice = vkDestroyDevice,
        .vkGetDeviceQueue = vkGetDeviceQueue,
    })
  , m_vmaFcts(VmaVulkanFunctions{

    })
{
    if (g_pMock)
    {
        throw logic_error("Cannot have more than one instance of MockVulkanLoader");
    }
    g_pMock = this;

    ON_CALL(*this, loadGlobalFcts()).WillByDefault(Return(m_gFcts));
    ON_CALL(*this, loadInstanceFcts(_)).WillByDefault(Return(m_iFcts));
    ON_CALL(*this, loadDeviceFcts(_)).WillByDefault(Return(m_dFcts));
    ON_CALL(*this, loadVmaFcts(_, _)).WillByDefault(Return(m_vmaFcts));
}

MockVulkanLoader::~MockVulkanLoader()
{
    g_pMock = nullptr;
}

auto MockVulkanLoader::createMockProxy() -> unique_ptr<IVulkanLoader>
{
    return make_unique<MockProxyVulkanLoader>(*this);
}