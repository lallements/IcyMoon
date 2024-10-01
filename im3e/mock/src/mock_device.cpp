#include "mock_device.h"

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

private:
    MockDevice& m_rMock;
};

}  // namespace

MockDevice::MockDevice() = default;
MockDevice::~MockDevice() = default;

auto MockDevice::createMockProxy() -> unique_ptr<IDevice>
{
    return make_unique<MockProxyDevice>(*this);
}