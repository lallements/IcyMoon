#include "anari_height_field_tile.h"

#include <im3e/utils/core/throw_utils.h>

using namespace im3e;

namespace {

auto createTileMaterial(AnariDevice& rAnDevice)
{
    auto pAnMaterial = rAnDevice.createMaterial(AnariMaterialType::Matte);

    const glm::vec3 color{0.8F, 0.2F, 0.2F};
    anariSetParameter(rAnDevice.getHandle(), pAnMaterial.get(), "color", ANARI_FLOAT32_VEC3, &color);

    anariCommitParameters(rAnDevice.getHandle(), pAnMaterial.get());
    return pAnMaterial;
}

auto mapVertexBuffer(AnariDevice& rAnDevice, ANARIGeometry anGeometry, const glm::u32vec2& rTileSize)
{
    constexpr auto VertexBufferKey = "vertex.position";

    const auto vertexCount = static_cast<uint64_t>(rTileSize.x) * static_cast<uint64_t>(rTileSize.y);
    uint64_t vertexStride{};
    auto* pData = anariMapParameterArray1D(rAnDevice.getHandle(), anGeometry, VertexBufferKey, ANARI_FLOAT32_VEC3,
                                           vertexCount, &vertexStride);
    throwIfNull<std::runtime_error>(pData, "Failed to map vertex buffer of ANARI height field tile");
    throwIfFalse<std::logic_error>(vertexStride == sizeof(glm::vec3), "Invalid vertex stride for ANARI height field");

    return UniquePtrWithDeleter<glm::vec3>{reinterpret_cast<glm::vec3*>(pData),
                                           [anDevice = rAnDevice.getHandle(), anGeometry](auto*) {
                                               anariUnmapParameterArray(anDevice, anGeometry, VertexBufferKey);
                                           }};
}

auto mapIndexBuffer(AnariDevice& rAnDevice, ANARIGeometry anGeometry, uint64_t triangleCount)
{
    constexpr auto IndexBufferKey = "primitive.index";

    uint64_t indexStride{};
    auto* pData = anariMapParameterArray1D(rAnDevice.getHandle(), anGeometry, IndexBufferKey, ANARI_UINT32_VEC3,
                                           triangleCount, &indexStride);
    throwIfNull<std::runtime_error>(pData, "Failed to map index buffer of ANARI height field tile");
    throwIfFalse<std::logic_error>(indexStride == sizeof(glm::u32vec3), "Invalid index stride for ANARI height field");

    return UniquePtrWithDeleter<glm::u32vec3>{reinterpret_cast<glm::u32vec3*>(pData),
                                              [anDevice = rAnDevice.getHandle(), anGeometry](auto*) {
                                                  anariUnmapParameterArray(anDevice, anGeometry, IndexBufferKey);
                                              }};
}

auto initializeIndexBuffer(AnariDevice& rAnDevice, ANARIGeometry anGeometry, const glm::u32vec2& rTileSize)
{
    // Each row of vertices generate 2 triangles per vertex except for the last vertex of each row i.e. 2 * (width - 1).
    // This applies to all rows except for the last one:
    //     (height - 1) * trianglesPerRow = (height - 1) * 2 * (width - 1) = 2 * (width - 1) * (height - 1)
    const auto triangleCount = 2U * static_cast<uint64_t>(rTileSize.x - 1U) * static_cast<uint64_t>(rTileSize.y - 1U);

    auto pIndices = mapIndexBuffer(rAnDevice, anGeometry, triangleCount);
    auto pIndexIt = pIndices.get();

    auto toIndex = [&rTileSize](uint32_t x, uint32_t y) { return rTileSize.x * y + x; };

    for (uint32_t y = 0U; y < rTileSize.y - 1U; y++)
    {
        for (uint32_t x = 0U; x < rTileSize.x - 1U; x++)
        {
            *(pIndexIt++) = glm::u32vec3{toIndex(x, y), toIndex(x, y + 1U), toIndex(x + 1U, y)};
            *(pIndexIt++) = glm::u32vec3{toIndex(x, y + 1U), toIndex(x + 1U, y + 1U), toIndex(x + 1U, y)};
        }
    }
}

auto initializeVertexBuffer(AnariDevice& rAnDevice, ANARIGeometry anGeometry, const glm::u32vec2& rTileSize)
{
    auto pVertices = mapVertexBuffer(rAnDevice, anGeometry, rTileSize);
    auto pVertexIt = pVertices.get();

    for (uint32_t y = 0U; y < rTileSize.y; y++)
    {
        for (uint32_t x = 0U; x < rTileSize.x; x++)
        {
            *(pVertexIt++) = glm::vec3{x, 0.0F, y};
        }
    }
}

}  // namespace

AnariHeightFieldTile::AnariHeightFieldTile(std::shared_ptr<AnariDevice> pAnDevice, const glm::u32vec2& rTileSize)
  : m_pAnDevice(throwIfArgNull(std::move(pAnDevice), "ANARI Height Field Tile requires a device"))
  , m_tileSize(rTileSize)

  , m_pAnGeometry(m_pAnDevice->createGeometry(AnariPrimitiveType::Triangle))
  , m_pAnMaterial(createTileMaterial(*m_pAnDevice))
  , m_pAnSurface(m_pAnDevice->createSurface(m_pAnGeometry.get(), m_pAnMaterial.get()))

  , m_pAnGroup(m_pAnDevice->createGroup({m_pAnSurface.get()}))
  , m_pAnInstance(m_pAnDevice->createInstance(m_pAnGroup.get()))
{
    initializeVertexBuffer(*m_pAnDevice, m_pAnGeometry.get(), m_tileSize);
    initializeIndexBuffer(*m_pAnDevice, m_pAnGeometry.get(), m_tileSize);
    anariCommitParameters(m_pAnDevice->getHandle(), m_pAnGeometry.get());
}

auto AnariHeightFieldTile::load(const IHeightMapTileSampler& rSampler) -> bool
{
    const auto scale = rSampler.getScale();
    const auto tilePos = glm::vec2(rSampler.getPos() * rSampler.getSize()) * scale;
    // const auto& rSize = rSampler.getSize();
    const auto& rActualSize = rSampler.getActualSize();

    // Prepare the vertex buffer first, the sampler sets invalid samples to NaN:
    auto pDstVertices = mapVertexBuffer(*m_pAnDevice, m_pAnGeometry.get(), rActualSize);
    auto pVertexIt = pDstVertices.get();
    auto setVertex = [&](uint32_t x, uint32_t y, float height) {
        *(pVertexIt + rActualSize.x * y + x) = glm::vec3{
            tilePos.x + x * scale,
            height,
            tilePos.y + y * scale,
        };
    };

    m_tmpIndices.clear();
    auto toIndex = [&rActualSize](uint32_t x, uint32_t y) { return rActualSize.x * y + x; };

    for (uint32_t y = 0U; y < rActualSize.y; y++)
    {
        for (uint32_t x = 0U; x < rActualSize.x; x++)
        {
            bool isCurrentValid = rSampler.isValid(x, y);
            if (isCurrentValid)
            {
                setVertex(x, y, rSampler.at(x, y));
            }

            if (!rSampler.isValid(x, y + 1U) || !rSampler.isValid(x + 1U, y))
            {
                continue;
            }

            if (isCurrentValid)
            {
                m_tmpIndices.emplace_back(toIndex(x, y), toIndex(x, y + 1U), toIndex(x + 1U, y));
            }

            if (rSampler.isValid(x + 1U, y + 1U))
            {
                m_tmpIndices.emplace_back(toIndex(x, y + 1U), toIndex(x + 1U, y + 1U), toIndex(x + 1U, y));
            }
        }
    }

    // If the index array is empty at this point, this means that the current tile did not contain any useful data to
    // load (e.g. if all the data is masked out). In this case, leave early and let the user know by returning false.
    if (m_tmpIndices.empty())
    {
        return false;
    }

    auto pDstIndices = mapIndexBuffer(*m_pAnDevice, m_pAnGeometry.get(), m_tmpIndices.size());
    std::ranges::copy(m_tmpIndices, pDstIndices.get());

    m_geometryChanged = true;
    return true;
}

void AnariHeightFieldTile::commitChanges()
{
    if (m_geometryChanged)
    {
        anariCommitParameters(m_pAnDevice->getHandle(), m_pAnGeometry.get());
        m_geometryChanged = false;
    }
}
