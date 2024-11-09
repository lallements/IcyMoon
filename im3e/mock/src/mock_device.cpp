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

    auto createLogger(string_view name) const -> unique_ptr<ILogger> override { return m_rMock.createLogger(name); }

    auto getVkInstance() const -> VkInstance override { return m_rMock.getVkInstance(); }
    auto getVkPhysicalDevice() const -> VkPhysicalDevice override { return m_rMock.getVkPhysicalDevice(); }
    auto getVkDevice() const -> VkDevice override { return m_rMock.getVkDevice(); }
    auto getFcts() const -> const VulkanDeviceFcts& override { return m_rMock.getFcts(); }
    auto getInstanceFcts() const -> const VulkanInstanceFcts& override { return m_rMock.getInstanceFcts(); }
    auto getImageFactory() const -> shared_ptr<const IImageFactory> override { return m_rMock.getImageFactory(); }
    auto getCommandQueue() const -> shared_ptr<const ICommandQueue> override { return m_rMock.getCommandQueue(); }
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
    ON_CALL(*this, getImageFactory()).WillByDefault(Invoke([this] { return m_mockImageFactory.createMockProxy(); }));
    ON_CALL(*this, getCommandQueue()).WillByDefault(Invoke([this] { return m_mockCommandQueue.createMockProxy(); }));
}

MockDevice::~MockDevice() = default;

auto MockDevice::createMockProxy() -> unique_ptr<IDevice>
{
    return make_unique<MockProxyDevice>(*this);
}