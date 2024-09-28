#pragma once

#include "logger_tracker.h"

#include <im3e/api/logger.h>
#include <im3e/utils/types.h>

#include <atomic>
#include <mutex>
#include <vector>

namespace im3e {

struct StreamLoggerContext
{
    std::atomic<LogLevel> levelFilter{LogLevel::Debug};

    std::mutex streamMutex;
    std::shared_ptr<std::ostream> pStream;

    std::mutex trackersMutex;
    std::vector<std::unique_ptr<LoggerTracker>> pTrackers;
};

class StreamLogger : public ILogger
{
public:
    StreamLogger(std::string_view category, std::shared_ptr<std::ostream> pStream);

    void setLevelFilter(LogLevel level) override;

    void error(std::string_view message) const override;
    void warning(std::string_view message) const override;
    void info(std::string_view message) const override;
    void debug(std::string_view message) const override;
    void verbose(std::string_view message) const override;

    auto createChild(std::string_view name) const -> std::unique_ptr<ILogger> override;
    auto createGlobalTracker() -> UniquePtrWithDeleter<ILoggerTracker> override;

private:
    StreamLogger(std::string_view name, std::shared_ptr<StreamLoggerContext> pContext);

    void _log(char type, LogLevel level, std::string_view message) const;

    const std::string m_name;
    const std::shared_ptr<StreamLoggerContext> m_pContext;
};

}  // namespace im3e