#pragma once

#include "test_utils.h"

#include <im3e/api/device.h>
#include <im3e/utils/loggers.h>
#include <im3e/utils/types.h>

#include <filesystem>
#include <memory>
#include <string>

namespace im3e {

/// @brief Test fixture base class for writing integration tests that require a device.
/// The class initializes a IDevice object as well as a ILogger that writes to a file of the same name as the test
/// suite. The device is created with debug enabled and the logger is tracked for any error messages. The test fails
/// if an error message is logged.
class DeviceIntegrationTest : public Test
{
public:
    DeviceIntegrationTest();

    void SetUp() override;
    void TearDown() override;

    static void SetUpTestSuite();
    static void TearDownTestSuite();

    /// @brief Generate a file path to save test output e.g. images
    /// @details The file path points to the test binary folder and is prefixed with the test suite name followed by
    /// the given name and extension.
    auto generateFilePath(std::string_view name, std::string_view extension) const -> std::filesystem::path;

    static auto getSuiteName() { return s_suiteName; }
    static auto getLogger() -> const ILogger& { return *s_pLogger; }
    static auto getDevice() { return s_pDevice; }
    auto getName() const { return m_name; }

private:
    static inline std::string s_suiteName{};
    static inline std::unique_ptr<ILogger> s_pLogger{};
    static inline UniquePtrWithDeleter<ILoggerTracker> s_pLoggerTracker{};
    static inline std::shared_ptr<IDevice> s_pDevice{};

    const std::string m_name;
    UniquePtrWithDeleter<void> m_pErrorTrackerScope;
};

}  // namespace im3e