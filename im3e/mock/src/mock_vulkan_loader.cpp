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

}  // namespace

MockVulkanGlobalFcts::MockVulkanGlobalFcts() = default;
MockVulkanGlobalFcts::~MockVulkanGlobalFcts() = default;

MockVulkanInstanceFcts::MockVulkanInstanceFcts() = default;
MockVulkanInstanceFcts::~MockVulkanInstanceFcts() = default;

MockVulkanDeviceFcts::MockVulkanDeviceFcts() = default;
MockVulkanDeviceFcts::~MockVulkanDeviceFcts() = default;

MockVmaVulkanFunctions::MockVmaVulkanFunctions() = default;
MockVmaVulkanFunctions::~MockVmaVulkanFunctions() = default;

MockVulkanLoader::MockVulkanLoader()
  : m_gFcts(VulkanGlobalFcts{
        .vkEnumerateInstanceVersion =
            [](uint32_t* pApiVersion) {
                assertMockExists();
                return g_pMock->getMockGlobalFcts().vkEnumerateInstanceVersion(pApiVersion);
            },
        .vkEnumerateInstanceExtensionProperties =
            [](const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
                assertMockExists();
                return g_pMock->getMockGlobalFcts().vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount,
                                                                                           pProperties);
            },
        .vkEnumerateInstanceLayerProperties =
            [](uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
                assertMockExists();
                return g_pMock->getMockGlobalFcts().vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
            },
        .vkCreateInstance =
            [](const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
               VkInstance* pInstance) {
                assertMockExists();
                return g_pMock->getMockGlobalFcts().vkCreateInstance(pCreateInfo, pAllocator, pInstance);
            },
    })
  , m_iFcts(VulkanInstanceFcts{
        .vkGetInstanceProcAddr = [](VkInstance instance, const char* pName) -> PFN_vkVoidFunction {
            assertMockExists();
            return g_pMock->getMockInstanceFcts().vkGetInstanceProcAddr(instance, pName);
        },
        .vkDestroyInstance =
            [](VkInstance instance, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockInstanceFcts().vkDestroyInstance(instance, pAllocator);
            },
        .vkCreateDevice =
            [](VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
               const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
                assertMockExists();
                return g_pMock->getMockInstanceFcts().vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
            },

        .vkCreateDebugUtilsMessengerEXT =
            [](VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
               const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
                assertMockExists();
                return g_pMock->getMockInstanceFcts().vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator,
                                                                                     pMessenger);
            },
        .vkDestroyDebugUtilsMessengerEXT =
            [](VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockInstanceFcts().vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
            },

        .vkEnumeratePhysicalDevices =
            [](VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
                assertMockExists();
                return g_pMock->getMockInstanceFcts().vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount,
                                                                                 pPhysicalDevices);
            },
        .vkGetPhysicalDeviceProperties =
            [](VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
                assertMockExists();
                g_pMock->getMockInstanceFcts().vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
            },
        .vkGetPhysicalDeviceFeatures =
            [](VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
                assertMockExists();
                g_pMock->getMockInstanceFcts().vkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
            },
        .vkEnumerateDeviceExtensionProperties =
            [](VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount,
               VkExtensionProperties* pProperties) {
                assertMockExists();
                return g_pMock->getMockInstanceFcts().vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName,
                                                                                           pPropertyCount, pProperties);
            },
        .vkGetPhysicalDeviceQueueFamilyProperties =
            [](VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
               VkQueueFamilyProperties* pQueueFamilyProperties) {
                assertMockExists();
                g_pMock->getMockInstanceFcts().vkGetPhysicalDeviceQueueFamilyProperties(
                    physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
            },
        .vkGetPhysicalDeviceMemoryProperties =
            [](VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
                assertMockExists();
                g_pMock->getMockInstanceFcts().vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
            },
    })
  , m_dFcts(VulkanDeviceFcts{
        .vkDestroyDevice =
            [](VkDevice device, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkDestroyDevice(device, pAllocator);
            },
        .vkGetDeviceQueue =
            [](VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
            },
        .vkQueueSubmit =
            [](VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
                assertMockExists();
                return g_pMock->getMockDeviceFcts().vkQueueSubmit(queue, submitCount, pSubmits, fence);
            },
        .vkGetImageSubresourceLayout =
            [](VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkGetImageSubresourceLayout(device, image, pSubresource, pLayout);
            },
        .vkCreateCommandPool =
            [](VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
               VkCommandPool* pCommandPool) {
                assertMockExists();
                return g_pMock->getMockDeviceFcts().vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
            },
        .vkDestroyCommandPool =
            [](VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkDestroyCommandPool(device, commandPool, pAllocator);
            },
        .vkAllocateCommandBuffers =
            [](VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) {
                assertMockExists();
                return g_pMock->getMockDeviceFcts().vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
            },
        .vkFreeCommandBuffers =
            [](VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
               const VkCommandBuffer* pCommandBuffers) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkFreeCommandBuffers(device, commandPool, commandBufferCount,
                                                                  pCommandBuffers);
            },
        .vkResetCommandBuffer =
            [](VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
                assertMockExists();
                return g_pMock->getMockDeviceFcts().vkResetCommandBuffer(commandBuffer, flags);
            },
        .vkBeginCommandBuffer =
            [](VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
                assertMockExists();
                return g_pMock->getMockDeviceFcts().vkBeginCommandBuffer(commandBuffer, pBeginInfo);
            },
        .vkEndCommandBuffer =
            [](VkCommandBuffer commandBuffer) {
                assertMockExists();
                return g_pMock->getMockDeviceFcts().vkEndCommandBuffer(commandBuffer);
            },
        .vkCmdPipelineBarrier2 =
            [](VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
            },
        .vkCmdClearColorImage =
            [](VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor,
               uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount,
                                                                  pRanges);
            },
        .vkCmdBeginRenderPass =
            [](VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
               VkSubpassContents contents) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, move(contents));
            },
        .vkCmdEndRenderPass =
            [](VkCommandBuffer commandBuffer) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkCmdEndRenderPass(commandBuffer);
            },

        .vkCreateFence =
            [](VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
               VkFence* pFence) {
                assertMockExists();
                return g_pMock->getMockDeviceFcts().vkCreateFence(device, pCreateInfo, pAllocator, pFence);
            },
        .vkDestroyFence =
            [](VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkDestroyFence(device, fence, pAllocator);
            },
        .vkWaitForFences =
            [](VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) {
                assertMockExists();
                return g_pMock->getMockDeviceFcts().vkWaitForFences(device, fenceCount, pFences, waitAll, timeout);
            },
        .vkResetFences =
            [](VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
                assertMockExists();
                return g_pMock->getMockDeviceFcts().vkResetFences(device, fenceCount, pFences);
            },

        .vkCreateFramebuffer = [](VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) -> VkResult {
            assertMockExists();
            return g_pMock->getMockDeviceFcts().vkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
        },
        .vkDestroyFramebuffer =
            [](VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkDestroyFramebuffer(device, framebuffer, pAllocator);
            },
        .vkCreateRenderPass = [](VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) -> VkResult {
            assertMockExists();
            return g_pMock->getMockDeviceFcts().vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        },
        .vkDestroyRenderPass =
            [](VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkDestroyRenderPass(device, renderPass, pAllocator);
            },
        .vkCreateDescriptorPool = [](VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator,
                                     VkDescriptorPool* pDescriptorPool) -> VkResult {
            assertMockExists();
            return g_pMock->getMockDeviceFcts().vkCreateDescriptorPool(device, pCreateInfo, pAllocator,
                                                                       pDescriptorPool);
        },
        .vkDestroyDescriptorPool =
            [](VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkDestroyDescriptorPool(device, descriptorPool, pAllocator);
            },
        .vkCreateImageView = [](VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkImageView* pView) -> VkResult {
            assertMockExists();
            return g_pMock->getMockDeviceFcts().vkCreateImageView(device, pCreateInfo, pAllocator, pView);
        },
        .vkDestroyImageView =
            [](VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockDeviceFcts().vkDestroyImageView(device, imageView, pAllocator);
            },
    })
  , m_vmaFcts(VmaVulkanFunctions{
        .vkGetPhysicalDeviceProperties =
            [](VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
            },
        .vkGetPhysicalDeviceMemoryProperties =
            [](VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
            },
        .vkAllocateMemory =
            [](VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator,
               VkDeviceMemory* pMemory) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
            },
        .vkFreeMemory =
            [](VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkFreeMemory(device, memory, pAllocator);
            },
        .vkMapMemory =
            [](VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags,
               void** ppData) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkMapMemory(device, memory, offset, size, flags, ppData);
            },
        .vkUnmapMemory =
            [](VkDevice device, VkDeviceMemory memory) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkUnmapMemory(device, memory);
            },
        .vkFlushMappedMemoryRanges =
            [](VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
            },
        .vkInvalidateMappedMemoryRanges =
            [](VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkInvalidateMappedMemoryRanges(device, memoryRangeCount,
                                                                                pMemoryRanges);
            },
        .vkBindBufferMemory =
            [](VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkBindBufferMemory(device, buffer, memory, memoryOffset);
            },
        .vkBindImageMemory =
            [](VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkBindImageMemory(device, image, memory, memoryOffset);
            },
        .vkGetBufferMemoryRequirements =
            [](VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
            },
        .vkGetImageMemoryRequirements =
            [](VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkGetImageMemoryRequirements(device, image, pMemoryRequirements);
            },
        .vkCreateBuffer =
            [](VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
               VkBuffer* pBuffer) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
            },
        .vkDestroyBuffer =
            [](VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkDestroyBuffer(device, buffer, pAllocator);
            },
        .vkCreateImage =
            [](VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
               VkImage* pImage) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkCreateImage(device, pCreateInfo, pAllocator, pImage);
            },
        .vkDestroyImage =
            [](VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkDestroyImage(device, image, pAllocator);
            },
        .vkCmdCopyBuffer =
            [](VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
               const VkBufferCopy* pRegions) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
            },
        .vkGetBufferMemoryRequirements2KHR =
            [](VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo,
               VkMemoryRequirements2* pMemoryRequirements) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
            },
        .vkGetImageMemoryRequirements2KHR =
            [](VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
               VkMemoryRequirements2* pMemoryRequirements) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
            },
        .vkBindBufferMemory2KHR =
            [](VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
            },
        .vkBindImageMemory2KHR =
            [](VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
                assertMockExists();
                return g_pMock->getMockVmaFcts().vkBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
            },
        .vkGetPhysicalDeviceMemoryProperties2KHR =
            [](VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
            },
        .vkGetDeviceBufferMemoryRequirements =
            [](VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo,
               VkMemoryRequirements2* pMemoryRequirements) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
            },
        .vkGetDeviceImageMemoryRequirements =
            [](VkDevice device, const VkDeviceImageMemoryRequirements* pInfo,
               VkMemoryRequirements2* pMemoryRequirements) {
                assertMockExists();
                g_pMock->getMockVmaFcts().vkGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
            },
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