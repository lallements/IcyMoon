#include <im3e/devices/devices.h>
#include <im3e/loggers/loggers.h>
#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

struct DeviceIntegration : public Test
{
    void TearDown() override { EXPECT_THAT(m_pLoggerTracker->getErrors(), IsEmpty()); }

    unique_ptr<ILogger> m_pLogger = createTerminalLogger();
    UniquePtrWithDeleter<ILoggerTracker> m_pLoggerTracker = m_pLogger->createGlobalTracker();
};

TEST_F(DeviceIntegration, constructor)
{
    auto pDevice = createDevice(*m_pLogger);
    ASSERT_THAT(pDevice, NotNull());
}

TEST_F(DeviceIntegration, constructorWithDebugEnabled)
{
    auto pDevice = createDevice(*m_pLogger, DeviceConfig{
                                                .isDebugEnabled = true,
                                            });
    ASSERT_THAT(pDevice, NotNull());
}