#include "src/glfw_window.h"

#include "src/glfw_window_application.h"
#include "src/imgui_workspace.h"

#include <im3e/devices/devices.h>
#include <im3e/test_utils/integration_test.h>
#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

struct GlfwWindowIntegrationTest : public IntegrationTest
{
    GlfwWindowIntegrationTest()
      : m_app(getLogger(), WindowApplicationConfig{.name = "TestApp", .isDebugEnabled = true})
    {
    }

    auto createWindow(shared_ptr<ImguiWorkspace> pWorkspace = make_shared<ImguiWorkspace>("workspace"))
    {
        return make_shared<GlfwWindow>(m_app.getDevice(), GlfwWindow::Config{}, pWorkspace);
    }

    GlfwWindowApplication m_app;
};

TEST_F(GlfwWindowIntegrationTest, constructor)
{
    auto pWindow = createWindow();
    ASSERT_THAT(pWindow, NotNull());
}

TEST_F(GlfwWindowIntegrationTest, resize)
{
    auto pWindow = createWindow();
    auto pGlfwWindow = pWindow->getHandle();

    pWindow->draw();

    glfwSetWindowSize(pGlfwWindow, 800U, 600U);
    glfwPollEvents();
    pWindow->draw();
}

TEST_F(GlfwWindowIntegrationTest, setImguiDemoVisible)
{
    auto pWorkspace = make_shared<ImguiWorkspace>("workspace");
    auto pWindow = createWindow(pWorkspace);

    for (auto i = 0U; i < 10U; i++)
    {
        if (i == 2U)
        {
            pWorkspace->setImguiDemoVisible(true);
        }

        glfwPollEvents();
        pWindow->draw();
    }
}