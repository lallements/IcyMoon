#pragma once

#include <im3e/api/logger.h>

namespace im3e {

class LoggerTracker : public ILoggerTracker
{
public:
    void addError(std::string_view message) { m_errors.emplace_back(message); }

    void clearErrors() override { m_errors.clear(); }

    auto getErrors() const -> std::vector<std::string> { return m_errors; }

private:
    std::vector<std::string> m_errors;
};

}  // namespace im3e