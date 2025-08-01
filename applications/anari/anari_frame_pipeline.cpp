#include "anari_frame_pipeline.h"

#include <im3e/utils/core/throw_utils.h>

using namespace im3e;
using namespace std;

namespace {

auto createFrame(const ILogger& rLogger, ANARIDevice anDevice, ANARIRenderer anRenderer, ANARICamera anCamera,
                 ANARIWorld anWorld, const VkExtent2D& rWindowSize)
{
    auto anFrame = anariNewFrame(anDevice);
    auto pFrame = UniquePtrWithDeleter<anari::api::Frame>(anFrame, [anDevice, pLogger = &rLogger](auto* anFrame) {
        anariRelease(anDevice, anFrame);
        pLogger->debug("Released frame");
    });

    const ANARIDataType format = ANARI_UFIXED8_VEC4;
    anariSetParameter(anDevice, anFrame, "size", ANARI_UINT32_VEC2, &rWindowSize);
    anariSetParameter(anDevice, anFrame, "channel.color", ANARI_DATA_TYPE, &format);
    anariSetParameter(anDevice, anFrame, "renderer", ANARI_RENDERER, &anRenderer);
    anariSetParameter(anDevice, anFrame, "camera", ANARI_CAMERA, &anCamera);
    anariSetParameter(anDevice, anFrame, "world", ANARI_WORLD, &anWorld);

    anariCommitParameters(anDevice, anFrame);

    rLogger.debug("Created frame");
    return pFrame;
}

void copyFrame(IStatsProvider& rStatsProvider, ANARIDevice anDevice, ANARIFrame anFrame, IHostVisibleImage& dstImage)
{
    auto pCopyFrameSpan = rStatsProvider.startScopedSpan("copyFrame");

    VkExtent2D srcSize{};
    ANARIDataType type = ANARI_UNKNOWN;
    auto* pSrcPixels = reinterpret_cast<const uint32_t*>(
        anariMapFrame(anDevice, anFrame, "channel.color", &srcSize.width, &srcSize.height, &type));
    {
        auto pVkImageMapping = dstImage.map();
        auto* pDstPixels = pVkImageMapping->getData();
        const auto dstRowPitch = pVkImageMapping->getRowPitch();
        for (auto row = 0U; row < srcSize.height; row++)
        {
            // In ANARI, images start from the bottom instead of the top:
            auto* pSrcRow = pSrcPixels + (srcSize.height - 1U - row) * srcSize.width;
            auto* pDstRow = reinterpret_cast<uint32_t*>(pDstPixels + row * dstRowPitch);
            copy(pSrcRow, pSrcRow + srcSize.width, pDstRow);
        }
        // pVkImageMapping->save("anari_output.png");
    }
    anariUnmapFrame(anDevice, anFrame, "channel.color");
}

inline void applyImageBarriersBeforeBlit(const ICommandBuffer& rCommandBuffer, IStatsProvider& rStatsProvider,
                                         IImage& rSrcImage, IImage& rDstImage)
{
    auto pFinalizeSpan = rStatsProvider.startScopedSpan("imageBarriers");

    auto pBarrier = rCommandBuffer.startScopedBarrier("finalizeOutputImage");
    pBarrier->addImageBarrier(rSrcImage, ImageBarrierConfig{
                                             .vkDstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
                                             .vkDstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
                                             .vkLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         });
    pBarrier->addImageBarrier(rDstImage, ImageBarrierConfig{
                                             .vkDstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
                                             .vkDstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                             .vkLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         });
}

inline void blitToOutputImage(const VulkanDeviceFcts& rFcts, const ICommandBuffer& rCommandBuffer,
                              IStatsProvider& rStatsProvider, IImage& rSrcImage, IImage& rDstImage)
{
    auto pBlitSpan = rStatsProvider.startScopedSpan("bitToOutput");

    applyImageBarriersBeforeBlit(rCommandBuffer, rStatsProvider, rSrcImage, rDstImage);

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

    rFcts.vkCmdBlitImage(rCommandBuffer.getVkCommandBuffer(), rSrcImage.getVkImage(),
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, rDstImage.getVkImage(),
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &vkImageBlit, VK_FILTER_NEAREST);
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

  , m_pCamera(make_shared<AnariMapCamera>(*m_pLogger, m_pAnDevice))
{
    m_pLogger->debug("Created Anari render panel");
}

void AnariFramePipeline::prepareExecution(const ICommandBuffer& rCommandBuffer, const VkExtent2D& rVkViewportSize,
                                          shared_ptr<IImage> pOutputImage)
{
    auto pStatsProvider = m_pDevice->getStatsProvider();
    auto pExecuteSpan = pStatsProvider->startScopedSpan("AnariFramePipeline.prepareExecution");

    if (!m_pAnFrame)
    {
        m_pLogger->error("Cannot render panel, no size provided!");
        return;
    }

    if (m_renderingFrame)
    {
        auto pPipelineSpan = pStatsProvider->startScopedSpan("outputFrame");
        {
            auto pFrameReadySpan = pStatsProvider->startScopedSpan("waitForFrame");
            anariFrameReady(m_pAnDevice.get(), m_pAnFrame.get(), ANARI_WAIT);
        }
        m_renderingFrame = false;

        if (m_currentViewportSize != rVkViewportSize)
        {
            auto pResizeSpan = pStatsProvider->startScopedSpan("resizeViewport");

            const auto aspectRatio = static_cast<float>(rVkViewportSize.width) /
                                     static_cast<float>(rVkViewportSize.height);
            m_pCamera->setAspectRatio(aspectRatio);

            anariSetParameter(m_pAnDevice.get(), m_pAnFrame.get(), "size", ANARI_UINT32_VEC2, &rVkViewportSize);
            anariCommitParameters(m_pAnDevice.get(), m_pAnFrame.get());

            m_currentViewportSize = rVkViewportSize;
        }
        {
            auto pCameraUpdateSpan = pStatsProvider->startScopedSpan("updateCamera");
            m_pCamera->update();
        }
        copyFrame(*pStatsProvider, m_pAnDevice.get(), m_pAnFrame.get(), *m_pImage);
        blitToOutputImage(m_pDevice->getFcts(), rCommandBuffer, *pStatsProvider, *m_pImage, *pOutputImage);
        {
            auto pBarrier = rCommandBuffer.startScopedBarrier("resetImageLayoutToGeneral");
            pBarrier->addImageBarrier(*m_pImage, ImageBarrierConfig{
                                                     .vkDstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                                                     .vkDstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                                                     .vkLayout = VK_IMAGE_LAYOUT_GENERAL,
                                                 });
        }
    }

    auto pRenderSpan = pStatsProvider->startScopedSpan("anariRenderFrame");
    anariRenderFrame(m_pAnDevice.get(), m_pAnFrame.get());
    m_renderingFrame = true;
}

void AnariFramePipeline::resize(const VkExtent2D& rVkExtent, uint32_t)
{
    m_pAnFrame.reset();
    m_pImage.reset();

    m_pAnFrame = createFrame(*m_pLogger, m_pAnDevice.get(), m_pAnRenderer.get(), m_pCamera->getHandle(),
                             m_pAnWorld.get(), rVkExtent);

    m_pImage = m_pDevice->getImageFactory()->createHostVisibleImage(ImageConfig{
        .name = "AnariPipelineImage",
        .vkExtent = rVkExtent,
        .vkFormat = VK_FORMAT_R8G8B8A8_UNORM,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    });
    {
        auto pCommandBuffer = m_pDevice->getCommandQueue()->startScopedCommand("AnariFramePipeline",
                                                                               CommandExecutionType::Sync);
        auto pBarrier = pCommandBuffer->startScopedBarrier("initializeImage");
        pBarrier->addImageBarrier(*m_pImage, ImageBarrierConfig{
                                                 .vkDstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                                                 .vkDstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                                                 .vkLayout = VK_IMAGE_LAYOUT_GENERAL,
                                             });
    }

    m_currentViewportSize = {};
}

auto AnariFramePipeline::getCameraListener() -> std::shared_ptr<IImguiEventListener>
{
    return m_pCamera;
}