#include "vulkan_images.h"

#include <im3e/utils/throw_utils.h>

using namespace im3e;
using namespace std;

namespace {

class ImageFactory : public IImageFactory
{
public:
    ImageFactory(shared_ptr<const IDevice> pDevice)
      : m_pDevice(throwIfArgNull(move(pDevice), "Image Factory requires a device"))
    {
    }

    auto createImage(ImageConfig) const -> unique_ptr<IImage> override { return nullptr; }
    auto createHostVisibleImage(ImageConfig) const -> unique_ptr<IHostVisibleImage> override { return nullptr; }

private:
    shared_ptr<const IDevice> m_pDevice;
};

}  // namespace

auto im3e::createVulkanImageFactory(shared_ptr<const IDevice> pDevice) -> unique_ptr<IImageFactory>
{
    return make_unique<ImageFactory>(move(pDevice));
}