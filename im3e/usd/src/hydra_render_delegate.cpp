#include "usd.h"

#include <im3e/utils/throw_utils.h>

#include <fmt/format.h>
#include <pxr/imaging/hd/driver.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginHandle.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgi/tokens.h>
#include <pxr/imaging/hgiGL/hgi.h>

using namespace im3e;
using namespace std;

namespace {

auto createHdStormRenderDelegate(const ILogger& rLogger)
{
    auto& rRendererPluginRegistry = pxr::HdRendererPluginRegistry::GetInstance();
    pxr::HfPluginDescVector pluginDescs;
    rRendererPluginRegistry.GetPluginDescs(&pluginDescs);
    throwIfFalse<runtime_error>(!pluginDescs.empty(), "Could not load any USD renderer plugin");

    auto& rPluginDesc = pluginDescs.front();
    rLogger.info(
        fmt::format(R"(Loading renderer plugin "{}" ({}))", rPluginDesc.displayName, rPluginDesc.id.GetString()));

    auto pRendererPlugin = rRendererPluginRegistry.GetOrCreateRendererPlugin(rPluginDesc.id);
    throwIfNull<runtime_error>(pRendererPlugin,
                               fmt::format("Failed to load USD renderer plugin \"{}\"", rPluginDesc.displayName));

    return unique_ptr<pxr::HdRenderDelegate>(pRendererPlugin->CreateRenderDelegate());
}

auto createRenderIndex(pxr::HdRenderDelegate* pRenderDelegate, pxr::HdDriver* pHdDriver)
{
    unique_ptr<pxr::HdRenderIndex> pRenderIndex(pxr::HdRenderIndex::New(pRenderDelegate, {pHdDriver}));
    throwIfNull<runtime_error>(pRenderIndex, "Failed to create Hydra render index");
    return pRenderIndex;
}

class HdStormRenderer : public IHydraRenderer
{
public:
    HdStormRenderer(shared_ptr<const IDevice> pDevice, shared_ptr<IGlContext> pGlContext)
      : m_pDevice(throwIfArgNull(move(pDevice), "Cannot create Hd Storm renderer without a device"))
      , m_pGlContext(throwIfArgNull(move(pGlContext), "Cannot create Hd Storm renderer without a GL context"))

      , m_pLogger(m_pDevice->createLogger("HdStorm"))
      , m_pRenderDelegate(createHdStormRenderDelegate(*m_pLogger))

      , m_pHgi(new pxr::HgiGL())
      , m_hdDriver(pxr::HdDriver{
            .name = pxr::HgiTokens->renderDriver,
            .driver = pxr::VtValue{m_pHgi.get()},
        })
      , m_pRenderIndex(createRenderIndex(m_pRenderDelegate.get(), &m_hdDriver))
    {
    }

    auto getRenderIndex() -> pxr::HdRenderIndex& override { return *m_pRenderIndex; }
    auto getRenderIndex() const -> const pxr::HdRenderIndex& override { return *m_pRenderIndex; }

private:
    shared_ptr<const IDevice> m_pDevice;
    shared_ptr<IGlContext> m_pGlContext;

    unique_ptr<ILogger> m_pLogger;
    unique_ptr<pxr::HdRenderDelegate> m_pRenderDelegate;

    pxr::HgiUniquePtr m_pHgi;
    pxr::HdDriver m_hdDriver;
    unique_ptr<pxr::HdRenderIndex> m_pRenderIndex;
};

}  // namespace

auto im3e::createHdStormRenderer(shared_ptr<const IDevice> pDevice, shared_ptr<IGlContext> pGlContext)
    -> shared_ptr<IHydraRenderer>
{
    return make_shared<HdStormRenderer>(move(pDevice), move(pGlContext));
}