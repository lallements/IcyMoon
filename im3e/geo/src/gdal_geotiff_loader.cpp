#include "gdal_geotiff_loader.h"

#include "gdal_utils.h"

#include <im3e/utils/throw_utils.h>

#include <fmt/format.h>
#include <fmt/std.h>

using namespace im3e;
using namespace std;

namespace {

template <typename T>
void printMetadata(const ILogger& rLogger, T& rMetadataHolder)
{
    auto pMetadataDomainList = rMetadataHolder.GetMetadataDomainList();
    auto metadataDomainCount = CSLCount(pMetadataDomainList);
    const vector<string> metadataDomains(pMetadataDomainList, pMetadataDomainList + metadataDomainCount);
    CSLDestroy(pMetadataDomainList);

    if (metadataDomains.empty())
    {
        rLogger.verbose(fmt::format("\t- Metadata: none"));
        return;
    }
    rLogger.verbose(fmt::format("\t- Metadata:"));

    uint32_t index{};
    for (const auto& rMetadataDomain : metadataDomains)
    {
        rLogger.verbose(fmt::format("\t\t- Domain[{}] = \"{}\"", index, rMetadataDomain));
        index++;

        auto pMetadata = rMetadataHolder.GetMetadata(rMetadataDomain.c_str());
        const auto metadataCount = CSLCount(pMetadata);
        vector<string> metadata(pMetadata, pMetadata + metadataCount);
        for (const auto& rMetadataKeyValue : metadata)
        {
            rLogger.verbose(fmt::format("\t\t\t--> {}", rMetadataKeyValue));
        }
    }
}

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

    rLogger.verbose(fmt::format("Information for file:"));
    rLogger.verbose(fmt::format("\t- Size: {}x{}", pDataset->GetRasterXSize(), pDataset->GetRasterYSize()));
    rLogger.verbose(fmt::format("\t- Raster count: {}", pDataset->GetRasterCount()));
    throwIfFalse<runtime_error>(pDataset->GetRasterCount() == 1U, "Unexpected number of raster bands, expected 1");

    rLogger.verbose(fmt::format("\t- Layer count: {}", pDataset->GetLayerCount()));
    rLogger.verbose(fmt::format("\t- Projection: {}", pDataset->GetProjectionRef()));

    printMetadata(rLogger, *pDataset);

    return pDataset;
}

void printRasterBandInfo(const ILogger& rLogger, GDALRasterBand& rRasterBand, string_view name)
{
    rLogger.verbose(fmt::format("Information for {}:", name));
    rLogger.verbose(fmt::format("\t- Data type: {}", convertGdalDataTypeToString(rRasterBand.GetRasterDataType())));
    rLogger.verbose(fmt::format("\t- Size: {}x{}", rRasterBand.GetXSize(), rRasterBand.GetYSize()));

    int blockSizeX, blockSizeY;
    rRasterBand.GetBlockSize(&blockSizeX, &blockSizeY);
    rLogger.verbose(fmt::format("\t- Block size: {}x{}", blockSizeX, blockSizeY));

    const auto blockCountX = (rRasterBand.GetXSize() + blockSizeX - 1) / blockSizeX;
    const auto blockCountY = (rRasterBand.GetYSize() + blockSizeY - 1) / blockSizeY;
    rLogger.verbose(fmt::format("\t- Block count: {}x{}", blockCountX, blockCountY));

    const auto suggestedAccessPatternStr = convertGdalSuggestedBlockAccessPatternToString(
        rRasterBand.GetSuggestedBlockAccessPattern());
    rLogger.verbose(fmt::format("\t- Suggested access pattern: {}", suggestedAccessPatternStr));

    rLogger.verbose(fmt::format("\t- Scale: {}", rRasterBand.GetScale()));
    rLogger.verbose(fmt::format("\t- Offset: {}", rRasterBand.GetOffset()));
    rLogger.verbose(fmt::format("\t- Unit Type: {}", rRasterBand.GetUnitType()));
    rLogger.verbose(fmt::format("\t- Overview count: {}", rRasterBand.GetOverviewCount()));
    rLogger.verbose(fmt::format("\t- Minimum: {}", rRasterBand.GetMinimum()));
    rLogger.verbose(fmt::format("\t- Maximum: {}", rRasterBand.GetMaximum()));

    int noDataValueFound{};
    const auto noDataValue = rRasterBand.GetNoDataValue(&noDataValueFound);
    if (noDataValueFound)
    {
        rLogger.verbose(fmt::format("\t- No Data Value: {}", noDataValue));
    }
    else
    {
        rLogger.verbose(fmt::format("\t- No Data Value: None"));
    }

    const auto maskFlags = rRasterBand.GetMaskFlags();
    rLogger.verbose(fmt::format("\t- Mask: {}", rRasterBand.GetMaskBand() ? "present" : "none"));
    rLogger.verbose(fmt::format("\t- Mask Flags: {} => {}", maskFlags, convertGdalMaskFlagsToString(maskFlags)));

    printMetadata(rLogger, rRasterBand);
}

auto loadRasterBand(const ILogger& rLogger, GDALDataset& rDataSet)
{
    auto pRasterBand = rDataSet.GetRasterBand(1);
    printRasterBandInfo(rLogger, *pRasterBand, "raster band");
    if (auto pMask = pRasterBand->GetMaskBand())
    {
        printRasterBandInfo(rLogger, *pMask, "raster band mask");
    }

    for (auto i = 0; i < pRasterBand->GetOverviewCount(); i++)
    {
        auto pOverview = pRasterBand->GetOverview(i);

        const string overviewName = fmt::format("overview[{}]", i);
        printRasterBandInfo(rLogger, *pOverview, overviewName);
        if (auto pMask = pRasterBand->GetMaskBand())
        {
            printRasterBandInfo(rLogger, *pMask, fmt::format("{} mask", overviewName));
        }
    }

    return pRasterBand;
}

}  // namespace

GdalGeoTiffHeightMap::GdalGeoTiffHeightMap(const ILogger& rLogger, HeightMapFileConfig config)
  : m_pLogger(rLogger.createChild(config.path.filename().string()))
  , m_config(move(config))
  , m_pGdalInstance(getGdalInstance(rLogger))
  , m_pDataset(createGdalDataset(*m_pLogger, m_config))
  , m_pRasterBand(loadRasterBand(*m_pLogger, *m_pDataset))
{
    m_pLogger->info("Successfully loaded file");
}

auto im3e::loadHeightMapFromFile(const ILogger& rLogger, HeightMapFileConfig config) -> unique_ptr<IHeightMap>
{
    return make_unique<GdalGeoTiffHeightMap>(rLogger, move(config));
}
