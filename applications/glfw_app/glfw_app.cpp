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
    pApp->createWindow();
    pApp->createWindow();

    pApp->run();
    return 0;
}