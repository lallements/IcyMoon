#include <im3e/devices/devices.h>
#include <im3e/loggers/loggers.h>
#include <im3e/test_utils/test_utils.h>

#include <filesystem>

using namespace im3e;
using namespace std;

struct VulkanDeviceIntegration : public Test
{
    void TearDown() override { EXPECT_THAT(m_pLoggerTracker->getErrors(), IsEmpty()); }

    static void SetUpTestSuite()
    {
        if (!s_pLogger)
        {
            auto* pTestSuite = testing::UnitTest::GetInstance()->current_test_suite();
            auto suiteName = string(pTestSuite->name());
            if (auto itFind = suiteName.find_first_of("/"); itFind != string::npos)
            {
                suiteName.erase(suiteName.find_first_of("/"));
            }

            s_pLogger = createFileLogger(filesystem::path{fmt::format("{}.log", suiteName)});
            s_pLogger->setLevelFilter(LogLevel::Debug);
        }
    }

    static inline unique_ptr<ILogger> s_pLogger;
    UniquePtrWithDeleter<ILoggerTracker> m_pLoggerTracker = s_pLogger->createGlobalTracker();
};

TEST_F(VulkanDeviceIntegration, constructor)
{
    auto pDevice = createDevice(*s_pLogger);
    ASSERT_THAT(pDevice, NotNull());
}

TEST_F(VulkanDeviceIntegration, constructorWithDebugEnabled)
{
    auto pDevice = createDevice(*s_pLogger, DeviceConfig{.isDebugEnabled = true});
    ASSERT_THAT(pDevice, NotNull());
}

TEST_F(VulkanDeviceIntegration, destroyDeviceBeforeDestroyingImage)
{
    auto pDevice = createDevice(*s_pLogger, DeviceConfig{.isDebugEnabled = true});
    auto pImage = pDevice->getImageFactory()->createImage(ImageConfig{
        .vkExtent{24U, 24U},
        .vkFormat = VK_FORMAT_R8G8B8A8_UNORM,
        .vkUsage = VK_IMAGE_USAGE_SAMPLED_BIT,
    });

    pDevice.reset();
}