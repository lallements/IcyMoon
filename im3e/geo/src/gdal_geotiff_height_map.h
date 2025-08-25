#pragma once

#include "gdal_instance.h"
#include "geo.h"

#include <im3e/api/height_map.h>
#include <im3e/utils/loggers.h>

#include <gdal_priv.h>

#include <memory>

namespace im3e {

class GdalGeoTiffHeightMap : public IHeightMap
{
public:
    GdalGeoTiffHeightMap(const ILogger& rLogger, HeightMapFileConfig config);

    auto getName() const -> std::string override { return m_config.path.stem().string(); }
    auto getTileSize() const -> glm::u32vec2 override { return m_tileSize; }

private:
    std::unique_ptr<ILogger> m_pLogger;
    const HeightMapFileConfig m_config;

    std::shared_ptr<IGdalInstance> m_pGdalInstance;
    GDALDatasetUniquePtr m_pDataset;
    GDALRasterBand* m_pRasterBand;

    const glm::u32vec2 m_tileSize;
};

}  // namespace im3e