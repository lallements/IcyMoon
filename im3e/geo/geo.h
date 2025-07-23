#pragma once

#include <im3e/api/height_map.h>
#include <im3e/utils/loggers.h>

#include <filesystem>

namespace im3e {

struct HeightMapFileConfig
{
    std::filesystem::path path;
    bool readOnly = true;
};
auto loadHeightMapFromFile(const ILogger& rLogger, HeightMapFileConfig config) -> std::unique_ptr<IHeightMap>;

}  // namespace im3e