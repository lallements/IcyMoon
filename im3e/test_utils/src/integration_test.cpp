#include "integration_test.h"

#include <im3e/devices/devices.h>
#include <im3e/loggers/loggers.h>

#include <fmt/format.h>

using namespace im3e;
using namespace std;
using namespace std::filesystem;

namespace {

constexpr auto TestsLogLevel = LogLevel::Debug;
constexpr bool VulkanValidationEnabled = true;

auto getTestName()
{
    auto* testInfo = testing::UnitTest::GetInstance()->current_test_info();
    const string name(testInfo->name());
    return name.substr(name.find_last_of("/") + 1U);
}

}  // namespace

IntegrationTest::IntegrationTest()
  : m_name([this] {
      const auto name = getTestName();
      s_pLogger->info(fmt::format("{:*^100}", name));
      return name;
  }())
{
}

void IntegrationTest::SetUp()
{
    EXPECT_THAT(s_pLoggerTracker->getErrors(), IsEmpty()) << "Start of error log tracking";
    s_pLoggerTracker->clearErrors();

    m_pErrorTrackerScope = UniquePtrWithDeleter<void>(reinterpret_cast<void*>(0x1234), [this](auto*) {
        EXPECT_THAT(s_pLoggerTracker->getErrors(), IsEmpty()) << "End of error log tracking";
        s_pLoggerTracker->clearErrors();
    });
}

void IntegrationTest::TearDown()
{
    m_pErrorTrackerScope.reset();
    s_pLogger->info(fmt::format("{:*>100}", ""));
}

void IntegrationTest::SetUpTestSuite()
{
    if (!s_pLogger)
    {
        auto* pTestSuite = testing::UnitTest::GetInstance()->current_test_suite();
        s_suiteName = string(pTestSuite->name());
        if (auto itFind = s_suiteName.find_first_of("/"); itFind != string::npos)
        {
            s_suiteName.erase(s_suiteName.find_first_of("/"));
        }

        s_pLogger = createFileLogger(path{fmt::format("{}.log", s_suiteName)});
        s_pLogger->setLevelFilter(TestsLogLevel);
        s_pLoggerTracker = s_pLogger->createGlobalTracker();
    }
}

void IntegrationTest::TearDownTestSuite()
{
    s_pLogger.reset();
}

auto IntegrationTest::generateFilePath(string_view name, string_view extension) const -> filesystem::path
{
    return filesystem::current_path() / filesystem::path{fmt::format("{}_{}.{}", getSuiteName(), name, extension)};
}