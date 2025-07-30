#include "stream_logger.h"

#include <im3e/utils/core/throw_utils.h>

#include <fmt/format.h>

#include <algorithm>
#include <fstream>
#include <iostream>

using namespace im3e;
using namespace std;
using namespace std::filesystem;

namespace {

constexpr auto RootName = "Root"sv;

auto addErrorToTrackers(StreamLoggerContext& rContext, string_view message)
{
    lock_guard<mutex> lg(rContext.trackersMutex);
    ranges::for_each(rContext.pTrackers, [&](auto& pTracker) { pTracker->addError(message); });
}

}  // namespace

StreamLogger::StreamLogger(string_view name, shared_ptr<ostream> pStream)
  : m_name(name)
  , m_pContext(make_shared<StreamLoggerContext>())
{
    m_pContext->pStream = throwIfArgNull(move(pStream), "Cannot create logger without stream");
}

void StreamLogger::setLevelFilter(LogLevel level)
{
    m_pContext->levelFilter.store(level);
}

void StreamLogger::error(string_view message) const
{
    _log('E', LogLevel::Error, message);

    addErrorToTrackers(*m_pContext, message);
}
void StreamLogger::warning(string_view message) const
{
    _log('W', LogLevel::Warning, message);
}
void StreamLogger::info(string_view message) const
{
    _log('I', LogLevel::Info, message);
}
void StreamLogger::debug(string_view message) const
{
    _log('D', LogLevel::Debug, message);
}
void StreamLogger::verbose(string_view message) const
{
    _log('V', LogLevel::Verbose, message);
}

auto StreamLogger::createChild(string_view name) const -> unique_ptr<ILogger>
{
    return unique_ptr<ILogger>(new StreamLogger(name, m_pContext));
}

auto StreamLogger::createGlobalTracker() -> UniquePtrWithDeleter<ILoggerTracker>
{
    lock_guard<mutex> lg(m_pContext->trackersMutex);
    m_pContext->pTrackers.emplace_back(make_unique<LoggerTracker>());

    return UniquePtrWithDeleter<ILoggerTracker>(m_pContext->pTrackers.back().get(), [pContext = m_pContext](auto* pT) {
        lock_guard<mutex> lg(pContext->trackersMutex);
        const auto [itStart, itEnd] = ranges::remove_if(pContext->pTrackers,
                                                        [&](auto& pTracker) { return pTracker.get() == pT; });
        pContext->pTrackers.erase(itStart, itEnd);
    });
}

StreamLogger::StreamLogger(string_view name, shared_ptr<StreamLoggerContext> pContext)
  : m_name(name)
  , m_pContext(move(pContext))
{
}

void StreamLogger::_log(char type, LogLevel level, string_view message) const
{
    if (level <= m_pContext->levelFilter.load())
    {
        lock_guard<mutex> lg(m_pContext->streamMutex);
        *(m_pContext->pStream) << fmt::format("[{}][{}] {}", type, m_name, message) << endl;
    }
}

unique_ptr<ILogger> im3e::createTerminalLogger()
{
    return make_unique<StreamLogger>(RootName, shared_ptr<ostream>(&cout, [](auto*) {}));
}

unique_ptr<ILogger> im3e::createFileLogger(const path& rFilePath)
{
    return make_unique<StreamLogger>(RootName, make_shared<ofstream>(rFilePath, ios::trunc));
}