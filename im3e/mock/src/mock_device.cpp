#include "mock_device.h"

#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

namespace {

class MockProxyDevice : public IDevice
{
public:
    MockProxyDevice(MockDevice& rMock)
      : m_rMock(rMock)
    {
    }

    auto getVkDevice() const -> VkDevice override { return m_rMock.getVkDevice(); }
    auto getVmaAllocator() const -> VmaAllocator override { return m_rMock.getVmaAllocator(); }
    auto getFcts() const -> const VulkanDeviceFcts& override { return m_rMock.getFcts(); }
    auto getImageFactory() const -> shared_ptr<const IImageFactory> override { return m_rMock.getImageFactory(); }

private:
    MockDevice& m_rMock;
};

auto createVmaAllocator(VkInstance vkInstance, VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice,
                        const VmaVulkanFunctions& rVmaVkFcts)
{
    VmaAllocatorCreateInfo vmaCreateInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT,
        .physicalDevice = vkPhysicalDevice,
        .device = vkDevice,
        .pVulkanFunctions = &rVmaVkFcts,
        .instance = vkInstance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };

    VmaAllocator vmaAllocator{};
    throwIfVkFailed(vmaCreateAllocator(&vmaCreateInfo, &vmaAllocator), "Could not create VMA allocator");

    return VkUniquePtr<VmaAllocator>(vmaAllocator, [](auto* pVmaAllocator) { vmaDestroyAllocator(pVmaAllocator); });
}

}  // namespace

MockDevice::MockDevice()
  : m_pVmaAllocator(createVmaAllocator(m_vkInstance, m_vkPhysicalDevice, m_vkDevice, m_mockVulkanLoader.getVmaFcts()))
{
    ON_CALL(*this, getVkDevice()).WillByDefault(Return(m_vkDevice));
    ON_CALL(*this, getImageFactory()).WillByDefault(Invoke([&] { return m_mockImageFactory.createMockProxy(); }));
    ON_CALL(*this, getFcts()).WillByDefault(Invoke([&] { return m_mockVulkanLoader.getDeviceFcts(); }));
    ON_CALL(*this, getVmaAllocator()).WillByDefault(Invoke(Return(m_pVmaAllocator.get())));
}

MockDevice::~MockDevice() = default;

auto MockDevice::createMockProxy() -> unique_ptr<IDevice>
{
    return make_unique<MockProxyDevice>(*this);
}