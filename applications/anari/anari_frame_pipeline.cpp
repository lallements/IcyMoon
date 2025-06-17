#include "anari_frame_pipeline.h"

#include <im3e/utils/throw_utils.h>

using namespace im3e;
using namespace std;

namespace {

auto createCamera(const ILogger& rLogger, ANARIDevice anDevice)
{
    auto anCamera = anariNewCamera(anDevice, "perspective");

    const array<float, 3U> position{};
    anariSetParameter(anDevice, anCamera, "position", ANARI_FLOAT32_VEC3, position.data());

    const array<float, 3U> up{0.0F, 1.0F, 0.0F};
    anariSetParameter(anDevice, anCamera, "up", ANARI_FLOAT32_VEC3, up.data());

    const array<float, 3U> direction{0.1F, 0.0F, 1.0F};
    anariSetParameter(anDevice, anCamera, "direction", ANARI_FLOAT32_VEC3, direction.data());

    anariCommitParameters(anDevice, anCamera);

    return UniquePtrWithDeleter<anari::api::Camera>(anCamera, [anDevice, pLogger = &rLogger](auto* anCamera) {
        anariRelease(anDevice, anCamera);
        pLogger->debug("Released camera");
    });
}

auto createFrame(const ILogger& rLogger, ANARIDevice anDevice, ANARIRenderer anRenderer, ANARICamera anCamera,
                 ANARIWorld anWorld, const VkExtent2D& rWindowSize)
{
    auto anFrame = anariNewFrame(anDevice);
    auto pFrame = UniquePtrWithDeleter<anari::api::Frame>(anFrame, [anDevice, pLogger = &rLogger](auto* anFrame) {
        anariRelease(anDevice, anFrame);
        pLogger->debug("Released frame");
    });

    const ANARIDataType format = ANARI_UFIXED8_RGBA_SRGB;
    anariSetParameter(anDevice, anFrame, "size", ANARI_UINT32_VEC2, &rWindowSize);
    anariSetParameter(anDevice, anFrame, "channel.color", ANARI_DATA_TYPE, &format);
    anariSetParameter(anDevice, anFrame, "renderer", ANARI_RENDERER, &anRenderer);
    anariSetParameter(anDevice, anFrame, "camera", ANARI_CAMERA, &anCamera);
    anariSetParameter(anDevice, anFrame, "world", ANARI_WORLD, &anWorld);

    anariCommitParameters(anDevice, anFrame);

    rLogger.debug("Created frame");
    return pFrame;
}

void copyFrame(const ILogger&, ANARIDevice anDevice, ANARIFrame anFrame, IHostVisibleImage& dstImage)
{
    array<uint32_t, 2U> size{};
    ANARIDataType type = ANARI_UNKNOWN;
    auto* pSrcPixels = reinterpret_cast<const uint32_t*>(
        anariMapFrame(anDevice, anFrame, "channel.color", &size[0], &size[1], &type));
    {
        auto pVkImageMapping = dstImage.map();
        auto* pDstPixels = reinterpret_cast<uint32_t*>(pVkImageMapping->getData());
        for (auto row = 0U; row < size[1]; row++)
        {
            auto* pSrcRow = pSrcPixels + (size[1] - 1 - row) * size[0];
            auto* pDstRow = pDstPixels + row * size[0];
            copy(pSrcRow, pSrcRow + size[0], pDstRow);
        }
        // pVkImageMapping->save("anari_output.png");
    }
    anariUnmapFrame(anDevice, anFrame, "channel.color");
}

inline void blitToOutputImage(const VulkanDeviceFcts& rFcts, VkCommandBuffer vkCommandBuffer, const IImage& rSrcImage,
                              IImage& rDstImage)
{
    const auto vkExtent = rDstImage.getVkExtent();

    const VkImageSubresourceLayers vkSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1U};
    const VkOffset3D vkStartOffset{};
    const VkOffset3D vkEndOffset{
        .x = vkStartOffset.x + static_cast<int32_t>(vkExtent.width),
        .y = vkStartOffset.y + static_cast<int32_t>(vkExtent.height),
        .z = 1,
    };

    VkImageBlit vkImageBlit{
        .srcSubresource = vkSubresourceLayers,
        .srcOffsets{vkStartOffset, vkEndOffset},
        .dstSubresource = vkSubresourceLayers,
        .dstOffsets{vkStartOffset, vkEndOffset},
    };

    rFcts.vkCmdBlitImage(vkCommandBuffer, rSrcImage.getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         rDstImage.getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &vkImageBlit,
                         VK_FILTER_NEAREST);
}

}  // namespace

AnariFramePipeline::AnariFramePipeline(const ILogger& rLogger, shared_ptr<IDevice> pDevice,
                                       shared_ptr<anari::api::Device> pAnDevice,
                                       shared_ptr<anari::api::Renderer> pAnRenderer,
                                       shared_ptr<anari::api::World> pAnWorld)
  : m_pLogger(rLogger.createChild("AnariRenderPanel"))
  , m_pDevice(throwIfArgNull(move(pDevice), "AnariRenderPanel requires a device"))
  , m_pAnDevice(throwIfArgNull(move(pAnDevice), "AnariRenderPanel requires an ANARI device"))
  , m_pAnRenderer(throwIfArgNull(move(pAnRenderer), "AnariRenderPanel requires an ANARI Renderer"))
  , m_pAnWorld(throwIfArgNull(move(pAnWorld), "AnariRenderPanel requires an ANARI World"))

  , m_pAnCamera(createCamera(*m_pLogger, m_pAnDevice.get()))
{
    m_pLogger->debug("Created Anari render panel");
}

void AnariFramePipeline::prepareExecution(const ICommandBuffer& rCommandBuffer, shared_ptr<IImage> pOutputImage)
{
    if (!m_pAnFrame)
    {
        m_pLogger->error("Cannot render panel, no size provided!");
        return;
    }

    if (m_currentViewportSize != rVkViewportSize)
    {
        const auto aspectRatio = static_cast<float>(rVkViewportSize.width) / static_cast<float>(rVkViewportSize.height);
        anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "aspect", ANARI_FLOAT32, &aspectRatio);
        anariCommitParameters(m_pAnDevice.get(), m_pAnCamera.get());

        anariSetParameter(m_pAnDevice.get(), m_pAnFrame.get(), "size", ANARI_UINT32_VEC2, &rVkViewportSize);
        anariCommitParameters(m_pAnDevice.get(), m_pAnFrame.get());

        m_currentViewportSize = rVkViewportSize;
    }

    anariRenderFrame(m_pAnDevice.get(), m_pAnFrame.get());
    anariFrameReady(m_pAnDevice.get(), m_pAnFrame.get(), ANARI_WAIT);

    {
        auto pCommandBuffer = m_pDevice->getCommandQueue()->startScopedCommand("AnariFramePipeline",
                                                                               CommandExecutionType::Sync);
        auto pBarrier = pCommandBuffer->startScopedBarrier("transitionLayoutBeforeCopy");
        pBarrier->addImageBarrier(*m_pImage, ImageBarrierConfig{
                                                 .vkDstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                                                 .vkDstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                                                 .vkLayout = VK_IMAGE_LAYOUT_GENERAL,
                                             });
    }
    copyFrame(*m_pLogger, m_pAnDevice.get(), m_pAnFrame.get(), *m_pImage);
    {
        auto pBarrier = rCommandBuffer.startScopedBarrier("finalizeOutputImage");
        pBarrier->addImageBarrier(*m_pImage, ImageBarrierConfig{
                                                 .vkDstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
                                                 .vkDstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
                                                 .vkLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                             });
        pBarrier->addImageBarrier(*pOutputImage, ImageBarrierConfig{
                                                     .vkDstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
                                                     .vkDstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                                     .vkLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                 });
    }
    blitToOutputImage(m_pDevice->getFcts(), rCommandBuffer.getVkCommandBuffer(), *m_pImage, *pOutputImage);
}

void AnariFramePipeline::resize(const VkExtent2D& rVkExtent, uint32_t)
{
    m_pAnFrame.reset();
    m_pImage.reset();

    const auto aspectRatio = static_cast<float>(rVkExtent.width) / static_cast<float>(rVkExtent.height);
    anariSetParameter(m_pAnDevice.get(), m_pAnCamera.get(), "aspect", ANARI_FLOAT32, &aspectRatio);
    anariCommitParameters(m_pAnDevice.get(), m_pAnCamera.get());

    m_pAnFrame = createFrame(*m_pLogger, m_pAnDevice.get(), m_pAnRenderer.get(), m_pAnCamera.get(), m_pAnWorld.get(),
                             rVkExtent);

    m_pImage = m_pDevice->getImageFactory()->createHostVisibleImage(ImageConfig{
        .name = "AnariPipelineImage",
        .vkExtent = rVkExtent,
        .vkFormat = VK_FORMAT_R8G8B8A8_SRGB,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    });

    m_currentViewportSize = {};
}
