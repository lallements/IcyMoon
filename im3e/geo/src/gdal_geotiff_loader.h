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

private:
    std::unique_ptr<ILogger> m_pLogger;
    const HeightMapFileConfig m_config;

    std::shared_ptr<IGdalInstance> m_pGdalInstance;
    GDALDatasetUniquePtr m_pDataset;
};

}  // namespace im3e