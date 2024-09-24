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

    auto loadGlobalFcts() -> VulkanGlobalFcts override { return m_rMock.loadGlobalFcts(); }
    auto loadInstanceFcts(VkInstance vkInstance) -> VulkanInstanceFcts override
    {
        return m_rMock.loadInstanceFcts(vkInstance);
    }
    auto loadDeviceFcts(VkDevice vkDevice) -> VulkanDeviceFcts override { return m_rMock.loadDeviceFcts(vkDevice); }

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

}  // namespace

MockGlobalFcts::MockGlobalFcts() = default;
MockGlobalFcts::~MockGlobalFcts() = default;

MockInstanceFcts::MockInstanceFcts() = default;
MockInstanceFcts::~MockInstanceFcts() = default;

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
    })
{
    if (g_pMock)
    {
        throw logic_error("Cannot have more than one instance of MockVulkanLoader");
    }
    g_pMock = this;

    ON_CALL(*this, loadGlobalFcts()).WillByDefault(Return(m_gFcts));
    ON_CALL(*this, loadInstanceFcts(_)).WillByDefault(Return(m_iFcts));
}

MockVulkanLoader::~MockVulkanLoader()
{
    g_pMock = nullptr;
}

auto MockVulkanLoader::createMockProxy() -> unique_ptr<IVulkanLoader>
{
    return make_unique<MockProxyVulkanLoader>(*this);
}