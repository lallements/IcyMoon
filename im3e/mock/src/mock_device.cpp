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

}  // namespace

MockDevice::MockDevice()
{
    ON_CALL(*this, getVkDevice()).WillByDefault(Return(m_vkDevice));
    ON_CALL(*this, getImageFactory()).WillByDefault(Invoke([&] { return m_mockImageFactory.createMockProxy(); }));
    ON_CALL(*this, getFcts()).WillByDefault(Invoke([&] { return m_mockVulkanLoader.getDeviceFcts(); }));
    ON_CALL(*this, getVmaAllocator()).WillByDefault(Invoke(Return(m_vmaAllocator)));
}

MockDevice::~MockDevice() = default;

auto MockDevice::createMockProxy() -> unique_ptr<IDevice>
{
    return make_unique<MockProxyDevice>(*this);
}