#include "vulkan_memory_allocator.h"

#include "vulkan_instance.h"

using namespace im3e;
using namespace std;

namespace {

auto createVmaAllocator(VkInstance vkInstance, VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice,
                        const VmaVulkanFunctions& rVmaVkFcts)
{
    VmaAllocatorCreateInfo vmaCreateInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT,
        .physicalDevice = vkPhysicalDevice,
        .device = vkDevice,
        .pVulkanFunctions = &rVmaVkFcts,
        .instance = vkInstance,
        .vulkanApiVersion = VulkanInstance::getVulkanApiVersion(),
    };

    VmaAllocator vmaAllocator{};
    throwIfVkFailed(vmaCreateAllocator(&vmaCreateInfo, &vmaAllocator), "Could not create VMA allocator");

    return VkUniquePtr<VmaAllocator>(vmaAllocator, [](auto* pVmaAllocator) { vmaDestroyAllocator(pVmaAllocator); });
}

class VulkanMemoryAllocator : public IMemoryAllocator
{
public:
    VulkanMemoryAllocator(shared_ptr<const IDevice> pDevice, VmaVulkanFunctions vmaFcts)
      : m_pDevice(throwIfArgNull(move(pDevice), "Vulkan memory allocator requires a device"))
      , m_vmaFcts(move(vmaFcts))
      , m_pVmaAllocator(createVmaAllocator(m_pDevice->getVkInstance(), m_pDevice->getVkPhysicalDevice(),
                                           m_pDevice->getVkDevice(), vmaFcts))
    {
    }

    auto createImage(const VkImageCreateInfo* pVkCreateInfo, const VmaAllocationCreateInfo* pVmaCreateInfo,
                     VkImage* pVkImage, VmaAllocation* pVmaAllocation, VmaAllocationInfo* pVmaAllocationInfo)
        -> VkResult override
    {
        return vmaCreateImage(m_pVmaAllocator.get(), pVkCreateInfo, pVmaCreateInfo, pVkImage, pVmaAllocation,
                              pVmaAllocationInfo);
    }

    void destroyImage(VkImage vkImage, VmaAllocation vmaAllocation) override
    {
        vmaDestroyImage(m_pVmaAllocator.get(), vkImage, vmaAllocation);
    }

private:
    shared_ptr<const IDevice> m_pDevice;
    VmaVulkanFunctions m_vmaFcts;
    VkUniquePtr<VmaAllocator> m_pVmaAllocator;
};

}  // namespace

auto im3e::createVulkanMemoryAllocator(shared_ptr<const IDevice> pDevice, VmaVulkanFunctions vmaFcts)
    -> unique_ptr<IMemoryAllocator>
{
    return make_unique<VulkanMemoryAllocator>(move(pDevice), move(vmaFcts));
}