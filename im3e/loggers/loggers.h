#pragma once

#include <im3e/api/logger.h>

#include <filesystem>

namespace im3e {

std::unique_ptr<ILogger> createTerminalLogger();
std::unique_ptr<ILogger> createFileLogger(const std::filesystem::path& rFilePath);

}  // namespace im3e