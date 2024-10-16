#include "mock_image.h"

using namespace im3e;
using namespace std;

namespace {

class MockProxyImageMetadata : public IImageMetadata
{
public:
    MockProxyImageMetadata(MockImageMetadata& rMock)
      : m_rMock(rMock)
    {
    }

    void setLayout(VkImageLayout vkLayout) override { m_rMock.setLayout(vkLayout); }
    void setLastStageMask(VkPipelineStageFlags2 vkStageMask) override { m_rMock.setLastStageMask(vkStageMask); }
    void setLastAccessMask(VkAccessFlags2 vkAccessMask) override { m_rMock.setLastAccessMask(vkAccessMask); }

    auto getLayout() const -> VkImageLayout override { return m_rMock.getLayout(); }
    auto getQueueFamilyIndex() const -> uint32_t override { return m_rMock.getQueueFamilyIndex(); }
    auto getLastStageMask() const -> VkPipelineStageFlags2 override { return m_rMock.getLastStageMask(); }
    auto getLastAccessMask() const -> VkAccessFlags2 override { return m_rMock.getLastAccessMask(); }

private:
    MockImageMetadata& m_rMock;
};

}  // namespace

MockImageMetadata::MockImageMetadata() = default;
MockImageMetadata::~MockImageMetadata() = default;

auto MockImageMetadata::createMockProxy() -> unique_ptr<IImageMetadata>
{
    return make_unique<MockProxyImageMetadata>(*this);
}

namespace {

class MockProxyImage : public IImage
{
public:
    MockProxyImage(MockImage& rMock)
      : m_rMock(rMock)
    {
    }

    auto getVkImage() const -> VkImage override { return m_rMock.getVkImage(); }
    auto getVkExtent() const -> VkExtent2D override { return m_rMock.getVkExtent(); }
    auto getFormat() const -> VkFormat override { return m_rMock.getFormat(); }
    auto getMetadata() -> shared_ptr<IImageMetadata> override { return m_rMock.getMetadata(); }
    auto getMetadata() const -> shared_ptr<const IImageMetadata> override { return m_rMock.getMetadata(); }

private:
    MockImage& m_rMock;
};

}  // namespace

MockImage::MockImage()
{
    ON_CALL(*this, getMetadata()).WillByDefault(Invoke([this] { return m_mockMetadata.createMockProxy(); }));
    ON_CALL(Const(*this), getMetadata()).WillByDefault(Invoke([this] { return m_mockMetadata.createMockProxy(); }));
}

MockImage::~MockImage() = default;

auto MockImage::createMockProxy() -> unique_ptr<IImage>
{
    return make_unique<MockProxyImage>(*this);
}

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
