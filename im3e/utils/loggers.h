#pragma once

#include "types.h"

#include <fmt/format.h>

#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

namespace im3e {

enum class LogLevel : uint8_t
{
    Error = 0U,
    Warning = 1U,
    Info = 2U,
    Debug = 3U,
    Verbose = 4U,
};

class ILoggerTracker
{
public:
    virtual ~ILoggerTracker() = default;

    virtual void clearErrors() = 0;

    virtual auto getErrors() const -> std::vector<std::string> = 0;
};

class ILogger
{
public:
    virtual ~ILogger() = default;

    virtual void setLevelFilter(LogLevel level) = 0;

    virtual void error(std::string_view message) const = 0;
    virtual void warning(std::string_view message) const = 0;
    virtual void info(std::string_view message) const = 0;
    virtual void debug(std::string_view message) const = 0;
    virtual void verbose(std::string_view message) const = 0;

    virtual auto createChild(std::string_view name) const -> std::unique_ptr<ILogger> = 0;

    // Tracks all log messages from current logger as well as all ancestors and descendants (see createChild()).
    virtual auto createGlobalTracker() -> UniquePtrWithDeleter<ILoggerTracker> = 0;
};

std::unique_ptr<ILogger> createTerminalLogger();
std::unique_ptr<ILogger> createFileLogger(const std::filesystem::path& rFilePath);

}  // namespace im3e