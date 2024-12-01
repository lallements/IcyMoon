#include "presenter.h"

#include <im3e/utils/vk_utils.h>

#include <algorithm>
#include <ranges>

using namespace im3e;
using namespace std;

namespace {

auto getVkSurfaceCapabilities(const IDevice& rDevice, VkSurfaceKHR vkSurface)
{
    const auto& rInstFcts = rDevice.getInstanceFcts();
    const auto vkPhysicalDevice = rDevice.getVkPhysicalDevice();

    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities{};
    throwIfVkFailed(
        rInstFcts.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &vkSurfaceCapabilities),
        "Could not query device surface capabilities for Presenter");
    return vkSurfaceCapabilities;
}

uint32_t determineImageCount(const VkSurfaceCapabilitiesKHR& vkCapabilities)
{
    constexpr auto TargetImageCount = 2U;

    auto imageCount = max(vkCapabilities.minImageCount, TargetImageCount);
    return vkCapabilities.maxImageCount == 0 ? imageCount : min(imageCount, vkCapabilities.maxImageCount);
}

VkSurfaceFormatKHR chooseSurfaceFormat(const ILogger& rLogger, const vector<VkSurfaceFormatKHR>& rVkSurfaceFormats)
{
    auto itFind = ranges::find_if(rVkSurfaceFormats, [](const auto& rVkSurfaceFormat) {
        return rVkSurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
               rVkSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    });

    if (itFind != rVkSurfaceFormats.end())
    {
        rLogger.info("Preferred format VK_FORMAT_B8G8R8A8_SRGB is supported");
        return *itFind;
    }

    throwIfFalse<invalid_argument>(!rVkSurfaceFormats.empty(), "No surface format is supported");
    rLogger.warning("Could not find preferred format, falling back to first available format");
    return rVkSurfaceFormats.front();
}

VkExtent2D chooseExtent(const ILogger& rLogger, const VkSurfaceCapabilitiesKHR& rVkCapabilities)
{
    const auto& rVkCurrentExtent = rVkCapabilities.currentExtent;
    const auto& rVkMaxExtent = rVkCapabilities.maxImageExtent;
    const auto maxValue = numeric_limits<uint32_t>::max();

    VkExtent2D vkExtent{
        .width = (rVkCurrentExtent.width == maxValue) ? rVkMaxExtent.width : rVkCurrentExtent.width,
        .height = (rVkCurrentExtent.height == maxValue) ? rVkMaxExtent.height : rVkCurrentExtent.height,
    };
    rLogger.info(fmt::format("Choosing extent {}x{} (max: {}x{})", vkExtent.width, vkExtent.height, rVkMaxExtent.width,
                             rVkMaxExtent.height));
    return vkExtent;
}

auto createVkSwapchainCreateInfo(const ILogger& rLogger, const IDevice& rDevice, VkSurfaceKHR vkSurface)
{
    throwIfNull<invalid_argument>(vkSurface, "WindowPresenter requires a surface");

    const auto& rInstFcts = rDevice.getInstanceFcts();
    const auto vkPhysicalDevice = rDevice.getVkPhysicalDevice();
    const auto vkSurfaceCapabilities = getVkSurfaceCapabilities(rDevice, vkSurface);
    const auto vkSurfaceFormats = getVkList<VkSurfaceFormatKHR>(rInstFcts.vkGetPhysicalDeviceSurfaceFormatsKHR,
                                                                "surface formats", vkPhysicalDevice, vkSurface);
    const auto vkPresentModes = getVkList<VkPresentModeKHR>(rInstFcts.vkGetPhysicalDeviceSurfacePresentModesKHR,
                                                            "present modes", vkPhysicalDevice, vkSurface);

    rLogger.debug(fmt::format("Device for target supports {} formats and {} present modes", vkSurfaceFormats.size(),
                              vkPresentModes.size()));

    const auto imageFormat = chooseSurfaceFormat(rLogger, vkSurfaceFormats);
    const auto queueFamilyIndex = rDevice.getCommandQueue()->getQueueFamilyIndex();

    return VkSwapchainCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vkSurface,
        .minImageCount = determineImageCount(vkSurfaceCapabilities),
        .imageFormat = imageFormat.format,
        .imageColorSpace = imageFormat.colorSpace,
        .imageExtent = chooseExtent(rLogger, vkSurfaceCapabilities),
        .imageArrayLayers = 1U,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1U,
        .pQueueFamilyIndices = &queueFamilyIndex,
        .preTransform = vkSurfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
}

auto createVkSwapchain(const IDevice& rDevice, const VkSwapchainCreateInfoKHR& vkCreateInfo)
{
    const auto vkDevice = rDevice.getVkDevice();
    const auto& rFcts = rDevice.getFcts();

    VkSwapchainKHR vkSwapchain{};
    throwIfVkFailed(rFcts.vkCreateSwapchainKHR(vkDevice, &vkCreateInfo, nullptr, &vkSwapchain),
                    "Could not create swapchain for Presenter");

    return makeVkUniquePtr<VkSwapchainKHR>(vkDevice, vkSwapchain, rFcts.vkDestroySwapchainKHR);
}

auto getSwapchainImages(const IDevice& rDevice, VkSwapchainKHR vkSwapchain,
                        const VkSwapchainCreateInfoKHR& rVkCreateInfo)
{
    auto vkImages = getVkList<VkImage>(rDevice.getFcts().vkGetSwapchainImagesKHR, "swapchain images",
                                       rDevice.getVkDevice(), vkSwapchain);

    auto pImageFactory = rDevice.getImageFactory();

    vector<shared_ptr<IImage>> pImages;
    pImages.reserve(vkImages.size());
    ranges::transform(vkImages, back_inserter(pImages), [&, n = 0](auto& rVkImage) mutable {
        auto pImage = pImageFactory->createProxyImage(rVkImage, ImageConfig{
                                                                    .name = fmt::format("SwapchainImage{}", n++),
                                                                    .vkExtent = rVkCreateInfo.imageExtent,
                                                                    .vkFormat = rVkCreateInfo.imageFormat,
                                                                    .vkUsage = rVkCreateInfo.imageUsage,
                                                                });
        return pImage;
    });
    return pImages;
}

VkPresentInfoKHR makeVkPresentInfo(const VkSwapchainKHR* ppSwapchain, uint32_t* pImageIndex,
                                   const VkSemaphore* ppFinalSemaphore)
{
    return VkPresentInfoKHR{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1U,
        .pWaitSemaphores = ppFinalSemaphore,
        .swapchainCount = 1U,
        .pSwapchains = ppSwapchain,
        .pImageIndices = pImageIndex,
        .pResults = nullptr,
    };
}

}  // namespace

Presenter::Presenter(shared_ptr<IDevice> pDevice, VkSurfaceKHR vkSurface, unique_ptr<IFramePipeline> pFramePipeline)
  : m_pDevice(throwIfArgNull(move(pDevice), "Presenter requires a device"))
  , m_vkSurface(throwIfArgNull(vkSurface, "Presenter requires a surface"))
  , m_pFramePipeline(throwIfArgNull(move(pFramePipeline), "Presenter requires a frame pipeline"))
  , m_pLogger(m_pDevice->createLogger("Presenter"))
{
    this->reset();
}

Presenter::~Presenter()
{
    // Wait for the queue to be idle as there might still be some pending vkQueuePresentKHR commands.
    m_pDevice->getCommandQueue()->waitIdle();
}

void Presenter::present()
{
    if (m_isOutOfDate)
    {
        m_pLogger->info("Swapchain currently out of date, a reset is needed");
        return;
    }

    const auto& rFcts = m_pDevice->getFcts();
    auto pCommandQueue = m_pDevice->getCommandQueue();

    m_semaphoreIndex = (m_semaphoreIndex + 1U) % m_pReadyToWriteSemaphores.size();
    auto pVkReadyToWriteSemaphore = m_pReadyToWriteSemaphores[m_semaphoreIndex];
    auto pVkReadyToPresentSemaphore = m_pReadyToPresentSemaphores[m_semaphoreIndex];
    uint32_t imageIndex{};
    {
        auto vkResult = rFcts.vkAcquireNextImageKHR(m_pDevice->getVkDevice(), m_pVkSwapchain.get(),
                                                    numeric_limits<uint64_t>::max(), pVkReadyToWriteSemaphore.get(),
                                                    nullptr, &imageIndex);
        if (vkResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_isOutOfDate = true;
            m_pLogger->info("swapchain out of date: will need reset");
            return;  // return early, we cannot do anything until the presenter is reset
        }
        else if (vkResult == VK_SUBOPTIMAL_KHR)
        {
            // We successfully acquired a new image so do not reset just yet, wait until all semaphores, etc. are in a
            // complete state.
            m_pLogger->info("Suboptimal: a reset will be needed after this draw call");
        }
        else
        {
            throwIfVkFailed(vkResult, "Failed to acquire next swapchain image for presenter");
        }
    }
    if (m_pImageFutures[imageIndex])
    {
        m_pImageFutures[imageIndex]->waitForCompletion();
        m_pImageFutures[imageIndex].reset();
    }
    {
        auto pCommandBuffer = pCommandQueue->startScopedCommand("present", CommandExecutionType::Async);
        pCommandBuffer->setVkWaitSemaphore(pVkReadyToWriteSemaphore);
        pCommandBuffer->setVkSignalSemaphore(pVkReadyToPresentSemaphore);

        m_pFramePipeline->prepareExecution(*pCommandBuffer, m_pImages[imageIndex]);
        {
            auto pBarrier = pCommandBuffer->startScopedBarrier("BeforePresentation");
            pBarrier->addImageBarrier(*m_pImages[imageIndex], ImageBarrierConfig{
                                                                  .vkLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                              });
        }

        m_pImageFutures[imageIndex] = pCommandBuffer->createFuture();
    }
    const auto vkSwapchain = m_pVkSwapchain.get();
    const auto vkReadyToPresentSemaphore = pVkReadyToPresentSemaphore.get();
    const auto vkPresentInfo = makeVkPresentInfo(&vkSwapchain, &imageIndex, &vkReadyToPresentSemaphore);
    auto vkPresentResult = rFcts.vkQueuePresentKHR(pCommandQueue->getVkQueue(), &vkPresentInfo);
    if (vkPresentResult == VK_ERROR_OUT_OF_DATE_KHR || vkPresentResult == VK_SUBOPTIMAL_KHR)
    {
        m_isOutOfDate = true;
        m_pLogger->info("Invalid state detected: a reset will be needed");
    }
    else
    {
        throwIfVkFailed(vkPresentResult, "Failed to present image");
    }
}

void Presenter::reset()
{
    // Wait for any future we have left to make sure our swapchain images have all been processed:
    ranges::for_each(m_pImageFutures, [](auto& pFuture) {
        if (pFuture)
        {
            pFuture->waitForCompletion();
        }
    });
    m_pImageFutures.clear();

    // Wait for the queue to be idle as there might still be images being presented via vkQueuePresentKHR and we
    // cannot have a fence for these calls:
    m_pDevice->getCommandQueue()->waitIdle();

    m_pVkSwapchain.reset();

    const auto vkCreateInfo = createVkSwapchainCreateInfo(*m_pLogger, *m_pDevice, m_vkSurface);
    m_pVkSwapchain = createVkSwapchain(*m_pDevice, vkCreateInfo);
    m_vkExtent = vkCreateInfo.imageExtent;
    m_pImages = getSwapchainImages(*m_pDevice, m_pVkSwapchain.get(), vkCreateInfo);
    m_pImageFutures.resize(m_pImages.size());

    // We add 1 one more set of sync objects than the total image count because we may try to acquire the next image
    // while all images have been queued. In this case, all sync object sets would also be used if we didn't have an
    // extra set.
    const auto syncObjectCount = m_pImages.size() + 1U;
    m_pReadyToWriteSemaphores.resize(syncObjectCount);
    m_pReadyToPresentSemaphores.resize(syncObjectCount);
    auto createVkSemaphore = [&](auto& pSemaphore) { pSemaphore = m_pDevice->createVkSemaphore(); };
    ranges::for_each(m_pReadyToWriteSemaphores, createVkSemaphore);
    ranges::for_each(m_pReadyToPresentSemaphores, createVkSemaphore);
    m_semaphoreIndex = {};

    m_pFramePipeline->resize(m_vkExtent, static_cast<uint32_t>(m_pImages.size()));

    m_isOutOfDate = false;
    m_pLogger->debug(fmt::format("Successfully initialized swapchain with {} images", m_pImages.size()));
}