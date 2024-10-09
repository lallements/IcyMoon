#include "mock_image.h"

using namespace im3e;
using namespace std;

namespace {

class MockProxyImageFactory : public IImageFactory
{
public:
    MockProxyImageFactory(MockImageFactory& rMock)
      : m_rMock(rMock)
    {
    }

    auto createImage(ImageConfig config) const -> unique_ptr<IImage> override
    {
        return m_rMock.createImage(move(config));
    }

    auto createHostVisibleImage(ImageConfig config) const -> unique_ptr<IHostVisibleImage> override
    {
        return m_rMock.createHostVisibleImage(move(config));
    }

private:
    MockImageFactory& m_rMock;
};

}  // namespace

MockImageFactory::MockImageFactory() = default;
MockImageFactory::~MockImageFactory() = default;

auto MockImageFactory::createMockProxy() -> unique_ptr<IImageFactory>
{
    return make_unique<MockProxyImageFactory>(*this);
}
