#include <im3e/guis/guis.h>
#include <im3e/usd/usd.h>
#include <im3e/utils/loggers.h>

#include <filesystem>

using namespace im3e;
using namespace std;

int main([[maybe_unused]] int argc, char* argv[])
{
    filesystem::path appPath{argv[0]};

    auto pLogger = createTerminalLogger();
    auto pApp = createGlfwWindowApplication(*pLogger, WindowApplicationConfig{
                                                          .name = appPath.filename(),
                                                          .isDebugEnabled = true,
                                                      });
    auto pDevice = pApp->getDevice();

    auto pWorkspace = createImguiWorkspace("Workspace");

    shared_ptr<IGuiPanel> pRenderPanel;
    {
        auto pRenderDelegate = createUsdRenderDelegate(pDevice);
        auto pHydraFramePipeline = createHydraFramePipeline(pDevice, move(pRenderDelegate));
        pRenderPanel = createImguiRenderPanel("Render", move(pHydraFramePipeline));
    }
    pWorkspace->addPanel(IGuiWorkspace::Location::Center, pRenderPanel, 0.5F);
    pApp->createWindow(pWorkspace);

    pApp->run();
    return 0;
}