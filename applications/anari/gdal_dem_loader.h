#pragma once

#include <im3e/utils/loggers.h>

#include <gdal_priv.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <memory>

namespace im3e {

class GdalDemLoader
{
public:
    GdalDemLoader(const ILogger& rLogger, const std::filesystem::path& rDemFilePath);

    auto getBlockSize() const -> const glm::u32vec2& { return m_blockSize; }
    auto getBlockCount() const -> const glm::u32vec2& { return m_blockCount; }
    auto getMinValue() const -> float { return m_minValue; }
    auto getBlockScale() const -> glm::vec3 { return m_blockScale; }

private:
    std::unique_ptr<ILogger> m_pLogger;
    GDALDatasetUniquePtr m_pDataset;
    GDALRasterBand* m_pRasterBand{};
    glm::u32vec2 m_blockSize;
    glm::u32vec2 m_blockCount;
    float m_minValue;
    glm::vec3 m_blockScale;
};

}  // namespace im3e