#include "usd.h"

#include <im3e/utils/throw_utils.h>

using namespace im3e;
using namespace std;

namespace {

class HydraFramePipeline : public IFramePipeline
{
public:
    HydraFramePipeline(std::shared_ptr<const IDevice> pDevice)
      : m_pDevice(throwIfArgNull(move(pDevice), "Hydra Frame Pipeline requires a device"))
    {
    }

    void prepareExecution(const ICommandBuffer&, std::shared_ptr<IImage>) override {}

    void resize(const VkExtent2D&, uint32_t) override {}

    auto getDevice() const -> std::shared_ptr<const IDevice> override { return m_pDevice; }

private:
    shared_ptr<const IDevice> m_pDevice;
};

}  // namespace

auto im3e::createUsdFramePipeline(shared_ptr<const IDevice> pDevice) -> unique_ptr<IFramePipeline>
{
    return make_unique<HydraFramePipeline>(move(pDevice));
}