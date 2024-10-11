#include "mock_device.h"

#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

namespace {

class MockProxyMemoryAllocator : public IMemoryAllocator
{
public:
    MockProxyMemoryAllocator(MockMemoryAllocator& rMock)
      : m_rMock(rMock)
    {
    }

    auto createImage(const VkImageCreateInfo* pVkCreateInfo, const VmaAllocationCreateInfo* pVmaCreateInfo,
                     VkImage* pVkImage, VmaAllocation* pVmaAllocation, VmaAllocationInfo* pVmaAllocationInfo)
        -> VkResult override
    {
        return m_rMock.createImage(pVkCreateInfo, pVmaCreateInfo, pVkImage, pVmaAllocation, pVmaAllocationInfo);
    }

    void destroyImage(VkImage vkImage, VmaAllocation vmaAllocation) override
    {
        m_rMock.destroyImage(vkImage, vmaAllocation);
    }

private:
    MockMemoryAllocator& m_rMock;
};

}  // namespace

MockMemoryAllocator::MockMemoryAllocator() = default;
MockMemoryAllocator::~MockMemoryAllocator() = default;

auto MockMemoryAllocator::createMockProxy() -> unique_ptr<IMemoryAllocator>
{
    return make_unique<MockProxyMemoryAllocator>(*this);
}

namespace {

class MockProxyDevice : public IDevice
{
public:
    MockProxyDevice(MockDevice& rMock)
      : m_rMock(rMock)
    {
    }

    auto createLogger(string_view name) const -> unique_ptr<ILogger> override { return m_rMock.createLogger(name); }

    auto getVkInstance() const -> VkInstance override { return m_rMock.getVkInstance(); }
    auto getVkPhysicalDevice() const -> VkPhysicalDevice override { return m_rMock.getVkPhysicalDevice(); }
    auto getVkDevice() const -> VkDevice override { return m_rMock.getVkDevice(); }
    auto getFcts() const -> const VulkanDeviceFcts& override { return m_rMock.getFcts(); }
    auto getMemoryAllocator() const -> shared_ptr<IMemoryAllocator> override { return m_rMock.getMemoryAllocator(); }
    auto getImageFactory() const -> shared_ptr<const IImageFactory> override { return m_rMock.getImageFactory(); }
    auto getCommandQueue() -> shared_ptr<ICommandQueue> override { return m_rMock.getCommandQueue(); }

private:
    MockDevice& m_rMock;
};

}  // namespace

MockDevice::MockDevice()
{
    ON_CALL(*this, createLogger(_)).WillByDefault(InvokeWithoutArgs([this] { return m_mockLogger.createMockProxy(); }));

    ON_CALL(*this, getVkInstance()).WillByDefault(Return(m_vkInstance));
    ON_CALL(*this, getVkPhysicalDevice()).WillByDefault(Return(m_vkPhysicalDevice));
    ON_CALL(*this, getVkDevice()).WillByDefault(Return(m_vkDevice));
    ON_CALL(*this, getFcts()).WillByDefault(ReturnRef(m_mockVulkanLoader.getDeviceFcts()));
    ON_CALL(*this, getMemoryAllocator()).WillByDefault(Invoke([this] {
        return m_mockMemoryAllocator.createMockProxy();
    }));
    ON_CALL(*this, getImageFactory()).WillByDefault(Invoke([this] { return m_mockImageFactory.createMockProxy(); }));
    ON_CALL(*this, getCommandQueue()).WillByDefault(Invoke([this] { return m_mockCommandQueue.createMockProxy(); }));
}

MockDevice::~MockDevice() = default;

auto MockDevice::createMockProxy() -> unique_ptr<IDevice>
{
    return make_unique<MockProxyDevice>(*this);
}