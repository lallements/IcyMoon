#pragma once

#include <im3e/api/vulkan_loader.h>
#include <im3e/test_utils/test_utils.h>

namespace im3e {

class MockGlobalFcts
{
public:
    MockGlobalFcts();
    virtual ~MockGlobalFcts();

    MOCK_METHOD(VkResult, vkEnumerateInstanceVersion, (uint32_t * pApiVersion));
    MOCK_METHOD(VkResult, vkEnumerateInstanceExtensionProperties,
                (const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties));
    MOCK_METHOD(VkResult, vkEnumerateInstanceLayerProperties,
                (uint32_t * pPropertyCount, VkLayerProperties* pProperties));
    MOCK_METHOD(VkResult, vkCreateInstance,
                (const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                 VkInstance* pInstance));
};

class MockInstanceFcts
{
public:
    MockInstanceFcts();
    virtual ~MockInstanceFcts();

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
};

class MockDeviceFcts
{
public:
    MockDeviceFcts();
    virtual ~MockDeviceFcts();

    MOCK_METHOD(void, vkDestroyDevice, (VkDevice device, const VkAllocationCallbacks* pAllocator));
};

class MockVulkanLoader : public IVulkanLoader
{
public:
    MockVulkanLoader();
    ~MockVulkanLoader() override;

    MOCK_METHOD(VulkanGlobalFcts, loadGlobalFcts, (), (override));
    MOCK_METHOD(VulkanInstanceFcts, loadInstanceFcts, (VkInstance vkInstance), (override));
    MOCK_METHOD(VulkanDeviceFcts, loadDeviceFcts, (VkDevice vkDevice), (override));

    auto createMockProxy() -> std::unique_ptr<IVulkanLoader>;

    auto getMockGlobalFcts() -> MockGlobalFcts& { return m_mockGFcts; }
    auto getMockInstanceFcts() -> MockInstanceFcts& { return m_mockIFcts; }
    auto getMockDeviceFcts() -> MockDeviceFcts& { return m_mockDFcts; }

private:
    VulkanGlobalFcts m_gFcts;
    VulkanInstanceFcts m_iFcts;
    VulkanDeviceFcts m_dFcts;

    NiceMock<MockGlobalFcts> m_mockGFcts;
    NiceMock<MockInstanceFcts> m_mockIFcts;
    NiceMock<MockDeviceFcts> m_mockDFcts;
};

}  // namespace im3e