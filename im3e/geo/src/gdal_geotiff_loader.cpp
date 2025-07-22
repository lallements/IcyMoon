#include "gdal_geotiff_loader.h"

#include <im3e/utils/throw_utils.h>

#include <fmt/format.h>
#include <fmt/std.h>

using namespace im3e;
using namespace std;

namespace {

auto createGdalDataset(const ILogger& rLogger, const HeightMapFileConfig& rConfig)
{
    // Required to silence warnings from LidarBC files due to mismatch in projection parameters from the file vs
    // ESPG standard. Here we set the option to GEOKEYS to prefer projection parameters from the local file.
    CPLSetConfigOption("GTIFF_SRS_SOURCE", "GEOKEYS");

    const auto fileAccess = rConfig.readOnly ? GA_ReadOnly : GA_Update;
    rLogger.debug(fmt::format("Reading file with {} access", rConfig.readOnly ? "read-only" : "read/write"));

    GDALDatasetUniquePtr pDataset(GDALDataset::FromHandle(GDALOpen(rConfig.path.c_str(), fileAccess)));
    throwIfNull<runtime_error>(pDataset, fmt::format("Failed to load height map file \"{}\"", rConfig.path));

    rLogger.debug(fmt::format(R"(Successfully loaded "{}" with driver "{}")", rConfig.path, pDataset->GetDriverName()));
    rLogger.debug(fmt::format("Raster size is {}x{}", pDataset->GetRasterXSize(), pDataset->GetRasterYSize()));
    rLogger.debug(fmt::format("Raster count is {}", pDataset->GetRasterCount()));
    rLogger.debug(fmt::format("Layer count is {}", pDataset->GetLayerCount()));
    rLogger.debug(fmt::format("Projection is {}", pDataset->GetProjectionRef()));

    return pDataset;
}

}  // namespace

GdalGeoTiffHeightMap::GdalGeoTiffHeightMap(const ILogger& rLogger, HeightMapFileConfig config)
  : m_pLogger(rLogger.createChild(config.path.filename().string()))
  , m_config(move(config))
  , m_pGdalInstance(getGdalInstance(rLogger))
  , m_pDataset(createGdalDataset(*m_pLogger, m_config))
{
    m_pLogger->info("Successfully loaded file");
}

auto im3e::loadHeightMapFromFile(const ILogger& rLogger, HeightMapFileConfig config) -> unique_ptr<IHeightMap>
{
    return make_unique<GdalGeoTiffHeightMap>(rLogger, move(config));
}
