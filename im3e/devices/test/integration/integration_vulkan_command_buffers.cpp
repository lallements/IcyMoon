#include <im3e/devices/devices.h>
#include <im3e/loggers/loggers.h>
#include <im3e/test_utils/test_utils.h>
#include <im3e/test_utils/vk.h>

using namespace im3e;
using namespace std;

struct VulkanCommandBuffersIntegration : public Test
{
    void TearDown() override { EXPECT_THAT(m_pLoggerTracker->getErrors(), ContainerEq(vector<string>{})); }

    unique_ptr<ILogger> m_pLogger = createTerminalLogger();
    UniquePtrWithDeleter<ILoggerTracker> m_pLoggerTracker = m_pLogger->createGlobalTracker();

    shared_ptr<IDevice> m_pDevice = createDevice(*m_pLogger, DeviceConfig{
                                                                 .isDebugEnabled = true,
                                                             });
    shared_ptr<const IImageFactory> m_pImageFactory = m_pDevice->getImageFactory();
};

TEST_F(VulkanCommandBuffersIntegration, constructor) {}
