#include "usd.h"

#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginHandle.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>

using namespace im3e;
using namespace std;

auto im3e::createUsdRenderDelegate(shared_ptr<const IDevice> pDevice) -> unique_ptr<pxr::HdRenderDelegate>
{
    throwIfArgNull(pDevice, "Cannot create USD render delegate without a device");
    auto pLogger = pDevice->createLogger("USD");

    auto& rRendererPluginRegistry = pxr::HdRendererPluginRegistry::GetInstance();
    pxr::HfPluginDescVector pluginDescs;
    rRendererPluginRegistry.GetPluginDescs(&pluginDescs);
    throwIfFalse<runtime_error>(!pluginDescs.empty(), "Could not load any USD renderer plugin");

    auto& rPluginDesc = pluginDescs.front();
    pLogger->info(
        fmt::format(R"(Loading renderer plugin "{}" ({}))", rPluginDesc.displayName, rPluginDesc.id.GetString()));

    auto pRendererPlugin = rRendererPluginRegistry.GetOrCreateRendererPlugin(rPluginDesc.id);
    throwIfNull<runtime_error>(pRendererPlugin,
                               fmt::format("Failed to load USD renderer plugin \"{}\"", rPluginDesc.displayName));

    return unique_ptr<pxr::HdRenderDelegate>(pRendererPlugin->CreateRenderDelegate());
}