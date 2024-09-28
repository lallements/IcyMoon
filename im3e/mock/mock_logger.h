#pragma once

#include <im3e/api/logger.h>
#include <im3e/test_utils/test_utils.h>

namespace im3e {

class MockLoggerTracker : public ILoggerTracker
{
public:
    MockLoggerTracker();
    ~MockLoggerTracker();

    MOCK_METHOD(void, clearErrors, (), (override));

    MOCK_METHOD(std::vector<std::string>, getErrors, (), (const, override));

    auto createMockProxy() -> std::unique_ptr<ILoggerTracker>;
};

class MockLogger : public ILogger
{
public:
    MockLogger();
    ~MockLogger() override;

    MOCK_METHOD(void, setLevelFilter, (LogLevel level), (override));

    MOCK_METHOD(void, error, (std::string_view message), (const, override));
    MOCK_METHOD(void, warning, (std::string_view message), (const, override));
    MOCK_METHOD(void, info, (std::string_view message), (const, override));
    MOCK_METHOD(void, debug, (std::string_view message), (const, override));
    MOCK_METHOD(void, verbose, (std::string_view message), (const, override));

    MOCK_METHOD(std::unique_ptr<ILogger>, createChild, (std::string_view category), (const, override));
    MOCK_METHOD(UniquePtrWithDeleter<ILoggerTracker>, createGlobalTracker, (), (override));

    auto createMockProxy() -> std::unique_ptr<ILogger>;

    auto getMockTracker() -> MockLoggerTracker& { return m_mockTracker; }

private:
    NiceMock<MockLoggerTracker> m_mockTracker;
};

}  // namespace im3e
