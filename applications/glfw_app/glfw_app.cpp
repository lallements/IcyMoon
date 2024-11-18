#include <im3e/guis/guis.h>
#include <im3e/loggers/loggers.h>

using namespace im3e;
using namespace std;

int main()
{
    auto pLogger = createTerminalLogger();
    auto pApp = createGlfwWindowApplication(*pLogger, WindowApplicationConfig{
                                                          .name = "GLFW App",
                                                          .isDebugEnabled = true,
                                                      });

    auto pWorkspace1 = createImguiWorkspace("Workspace 1");
    pApp->createWindow(pWorkspace1);

    // auto pWorkspace2 = createImguiWorkspace("Workspace 2");
    // pApp->createWindow(pWorkspace2);

    pApp->run();
    return 0;
}