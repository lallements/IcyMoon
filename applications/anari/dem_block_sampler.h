#pragma once

#include <gdal_priv.h>
#include <glm/glm.hpp>

#include <functional>
#include <memory>

namespace im3e {

template <typename T>
class DemBlockSampler
{
public:
    DemBlockSampler(std::unique_ptr<const T, std::function<void(const T*)>> pData, glm::u32vec2 blockPos,
                    glm::u32vec2 blockSize, glm::u32vec2 actualBlockSize, T minValue, T scale)
      : m_pData(std::move(pData))
      , m_blockPos(std::move(blockPos))
      , m_actualBlockSize(std::move(actualBlockSize))
      , m_blockSize(std::move(blockSize))
      , m_minValue(minValue)
      , m_scale(scale)
    {
    }

    auto at(uint32_t x, uint32_t y) const -> T { return m_pData.get()[m_blockSize.x * y + x]; }
    auto at(const glm::u32vec2& rPos) const -> T { return m_pData.get()[m_blockSize.x * rPos.y + rPos.x]; }

    auto getBlockPos() const -> const glm::u32vec2& { return m_blockPos; }
    auto getActualBlockSize() const -> const glm::u32vec2& { return m_actualBlockSize; }
    auto getBlockSize() const -> const glm::u32vec2& { return m_blockSize; }

private:
    std::unique_ptr<const T, std::function<void(const T*)>> m_pData;
    const glm::u32vec2 m_blockPos{};
    const glm::u32vec2 m_actualBlockSize{};
    const glm::u32vec2 m_blockSize{};
    const T m_minValue{};
    const T m_scale{};
};

template <typename T>
struct DemBlockSamplers
{
    std::unique_ptr<DemBlockSampler<T>> pBlock;
    std::unique_ptr<DemBlockSampler<T>> pTopBlock;
    std::unique_ptr<DemBlockSampler<T>> pTopLeftBlock;
    std::unique_ptr<DemBlockSampler<T>> pLeftBlock;
    std::unique_ptr<DemBlockSampler<T>> pBottomLeftBlock;
    std::unique_ptr<DemBlockSampler<T>> pBottomBlock;
    std::unique_ptr<DemBlockSampler<T>> pBottomRightBlock;
    std::unique_ptr<DemBlockSampler<T>> pRightBlock;
    std::unique_ptr<DemBlockSampler<T>> pTopRightBlock;
};

}  // namespace im3e