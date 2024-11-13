#include "glfw_window_application.h"

#include "guis.h"

#include <im3e/devices/devices.h>

#include <GLFW/glfw3.h>
#include <fmt/format.h>

#include <algorithm>

using namespace im3e;
using namespace std;

GlfwInstance::GlfwInstance(const ILogger& rLogger)
  : m_rLogger(rLogger)
{
    throwIfFalse<runtime_error>(glfwInit() == GLFW_TRUE, "Failed to initialize GLFW");
    throwIfFalse<runtime_error>(glfwVulkanSupported() == GLFW_TRUE, "GLFW does not support Vulkan on this setup");
    m_rLogger.debug("Successfully initialized GLFW with Vulkan support");
}

GlfwInstance::~GlfwInstance()
{
    glfwTerminate();
    m_rLogger.debug("Successfully terminated GLFW");
}

GlfwWindowApplication::GlfwWindowApplication(const ILogger& rLogger, WindowApplicationConfig config)
  : m_config(move(config))
  , m_pLogger(rLogger.createChild(m_config.name))
  , m_pGlfwInstance(make_unique<GlfwInstance>(*m_pLogger))
  , m_pDevice(createDevice(*m_pLogger, DeviceConfig{
                                           .isDebugEnabled = false,
                                           .isPresentationSupported = glfwGetPhysicalDevicePresentationSupport,
                                       }))
{
}

void GlfwWindowApplication::createWindow()
{
    const auto windowName = fmt::format("{} - Window #{}", m_config.name, m_pWindows.size());
    m_pWindows.emplace_back(make_unique<GlfwWindow>(m_pDevice, windowName));
}

void GlfwWindowApplication::run()
{
    while (!m_pWindows.empty())
    {
        auto itWindow = m_pWindows.begin();
        while (itWindow != m_pWindows.end())
        {
            if ((*itWindow)->shouldClose())
            {
                itWindow = m_pWindows.erase(itWindow);
                continue;
            }

            glfwPollEvents();
            (*itWindow)->draw();

            itWindow++;
        }

        // If all windows are minimized, we should block the loop until an event wakes us up to avoid entering a busy
        // loop:
        if (ranges::all_of(m_pWindows, [](auto& pWindow) { return pWindow->isIconified(); }))
        {
            glfwWaitEvents();
        }
    }
}

auto im3e::createGlfwWindowApplication(const ILogger& rLogger, WindowApplicationConfig config)
    -> shared_ptr<IWindowApplication>
{
    return make_unique<GlfwWindowApplication>(rLogger, move(config));
}