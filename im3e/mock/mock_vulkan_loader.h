#pragma once

#include <im3e/api/vulkan_loader.h>
#include <im3e/test_utils/test_utils.h>

namespace im3e {

class MockVulkanGlobalFcts
{
public:
    MockVulkanGlobalFcts();
    virtual ~MockVulkanGlobalFcts();

    MOCK_METHOD(VkResult, vkEnumerateInstanceVersion, (uint32_t * pApiVersion));
    MOCK_METHOD(VkResult, vkEnumerateInstanceExtensionProperties,
                (const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties));
    MOCK_METHOD(VkResult, vkEnumerateInstanceLayerProperties,
                (uint32_t * pPropertyCount, VkLayerProperties* pProperties));
    MOCK_METHOD(VkResult, vkCreateInstance,
                (const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                 VkInstance* pInstance));
};

class MockVulkanInstanceFcts
{
public:
    MockVulkanInstanceFcts();
    virtual ~MockVulkanInstanceFcts();

    MOCK_METHOD(void, vkDestroyInstance, (VkInstance instance, const VkAllocationCallbacks* pAllocator));
    MOCK_METHOD(VkResult, vkCreateDevice,
                (VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                 const VkAllocationCallbacks* pAllocator, VkDevice* pDevice));

    MOCK_METHOD(VkResult, vkCreateDebugUtilsMessengerEXT,
                (VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                 const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger));
    MOCK_METHOD(void, vkDestroyDebugUtilsMessengerEXT,
                (VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator));

    MOCK_METHOD(VkResult, vkEnumeratePhysicalDevices,
                (VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices));
    MOCK_METHOD(void, vkGetPhysicalDeviceProperties,
                (VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties));
    MOCK_METHOD(void, vkGetPhysicalDeviceFeatures,
                (VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures));
    MOCK_METHOD(VkResult, vkEnumerateDeviceExtensionProperties,
                (VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount,
                 VkExtensionProperties* pProperties));
    MOCK_METHOD(void, vkGetPhysicalDeviceQueueFamilyProperties,
                (VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                 VkQueueFamilyProperties* pQueueFamilyProperties));
    MOCK_METHOD(void, vkGetPhysicalDeviceMemoryProperties,
                (VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties));
};

class MockVulkanDeviceFcts
{
public:
    MockVulkanDeviceFcts();
    virtual ~MockVulkanDeviceFcts();

    MOCK_METHOD(void, vkDestroyDevice, (VkDevice device, const VkAllocationCallbacks* pAllocator));
    MOCK_METHOD(void, vkGetDeviceQueue,
                (VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue));
    MOCK_METHOD(void, vkGetImageSubresourceLayout,
                (VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout));
    MOCK_METHOD(VkResult, vkMapMemory,
                (VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags,
                 void** ppData));
    MOCK_METHOD(void, vkUnmapMemory, (VkDevice device, VkDeviceMemory memory));
};

class MockVulkanLoader : public IVulkanLoader
{
public:
    MockVulkanLoader();
    ~MockVulkanLoader() override;

    MOCK_METHOD(VulkanGlobalFcts, loadGlobalFcts, (), (const, override));
    MOCK_METHOD(VulkanInstanceFcts, loadInstanceFcts, (VkInstance vkInstance), (const, override));
    MOCK_METHOD(VulkanDeviceFcts, loadDeviceFcts, (VkDevice vkDevice), (const, override));
    MOCK_METHOD(VmaVulkanFunctions, loadVmaFcts, (VkInstance vkInstance, VkDevice vkDevice), (const, override));

    auto createMockProxy() -> std::unique_ptr<IVulkanLoader>;

    auto getInstanceFcts() const -> const VulkanInstanceFcts& { return m_iFcts; }
    auto getDeviceFcts() const -> const VulkanDeviceFcts& { return m_dFcts; }

    auto getMockGlobalFcts() -> MockVulkanGlobalFcts& { return m_mockGFcts; }
    auto getMockInstanceFcts() -> MockVulkanInstanceFcts& { return m_mockIFcts; }
    auto getMockDeviceFcts() -> MockVulkanDeviceFcts& { return m_mockDFcts; }

private:
    VulkanGlobalFcts m_gFcts;
    VulkanInstanceFcts m_iFcts;
    VulkanDeviceFcts m_dFcts;
    VmaVulkanFunctions m_vmaFcts;

    NiceMock<MockVulkanGlobalFcts> m_mockGFcts;
    NiceMock<MockVulkanInstanceFcts> m_mockIFcts;
    NiceMock<MockVulkanDeviceFcts> m_mockDFcts;
};

}  // namespace im3e