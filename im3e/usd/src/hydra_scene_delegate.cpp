#include "usd.h"

#include <pxr/usd/usd/stage.h>

using namespace im3e;
using namespace std;

auto im3e::openUsdStage(const filesystem::path& rFilePath) -> pxr::UsdStageRefPtr
{
    throwIfFalse<invalid_argument>(
        filesystem::exists(rFilePath),
        fmt::format(R"(Cannot open USD scene: file "{}" does not exist)", rFilePath.string()));

    auto pStage = pxr::UsdStage::Open(rFilePath.string());
    throwIfNull<runtime_error>(pStage, fmt::format(R"(Failed to open USD stage in file "{}")", rFilePath.string()));
    return pStage;
}
