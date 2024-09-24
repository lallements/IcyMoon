#include "vulkan_extensions.h"

#include <im3e/utils/vk_utils.h>

#include <fmt/format.h>

#include <algorithm>
#include <ranges>
#include <string_view>

using namespace im3e;
using namespace std;

namespace {

void insertIfUnique(string_view item, vector<string_view>& rItems)
{
    if (ranges::find(rItems, item) == rItems.end())
    {
        rItems.push_back(item);
    }
}

auto generateDeviceExtensions()
{
    vector<string_view> deviceExtensions;

    // For ray tracing:
    insertIfUnique(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, deviceExtensions);
    insertIfUnique(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, deviceExtensions);
    insertIfUnique(VK_KHR_RAY_QUERY_EXTENSION_NAME, deviceExtensions);
    insertIfUnique(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, deviceExtensions);

    insertIfUnique(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, deviceExtensions);

    return deviceExtensions;
}

void addLayers(bool isVkValidationEnabled, vector<string_view>& rLayers)
{
    if (isVkValidationEnabled)
    {
        insertIfUnique("VK_LAYER_KHRONOS_validation", rLayers);
    }
}

void addInstanceExtensions(bool isVkValidationEnabled, vector<string_view>& rExtensions)
{
    if (isVkValidationEnabled)
    {
        insertIfUnique(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, rExtensions);
    }
}

auto generateInstanceExtensions(const ILogger& logger, const VulkanGlobalFcts& rFcts, bool isVkValidationEnabled,
                                const vector<string_view>& rRequiredExtensions)
{
    vector<string_view> extensions;
    ranges::for_each(rRequiredExtensions, [&](auto& rExtension) { insertIfUnique(rExtension, extensions); });
    addInstanceExtensions(isVkValidationEnabled, extensions);

    logger.info("Checking support for required instance extensions");
    const auto supportedExtensions = getVkList<VkExtensionProperties>(rFcts.vkEnumerateInstanceExtensionProperties,
                                                                      "supported instance extensions", nullptr);

    bool allSupported = true;
    for (auto& rExtension : extensions)
    {
        auto itFind = ranges::find_if(supportedExtensions, [&](const auto& props) {
            return strcmp(rExtension.data(), &props.extensionName[0]) == 0;
        });

        if (itFind != supportedExtensions.end())
        {
            logger.info(fmt::format("OK - {} v{}", itFind->extensionName, itFind->specVersion));
        }
        else
        {
            logger.info(fmt::format("FAILED - {}", rExtension));
            allSupported = false;
        }
    }
    throwIfFalse<runtime_error>(allSupported, "Some required instance extensions are not supported");

    return extensions;
}

auto generateLayers(const ILogger& logger, const VulkanGlobalFcts& rFcts, bool isVkValidationEnabled)
{
    vector<string_view> layers;
    addLayers(isVkValidationEnabled, layers);

    logger.info("Checking support for required layers");
    const auto supportedLayers = getVkList<VkLayerProperties>(rFcts.vkEnumerateInstanceLayerProperties,
                                                              "supported layers");

    bool allSupported = true;
    auto itLayer = layers.begin();
    while (itLayer != layers.end())
    {
        auto itFind = ranges::find_if(
            supportedLayers, [&](const auto& props) { return strcmp(itLayer->data(), &props.layerName[0]) == 0; });

        if (itFind != supportedLayers.end())
        {
            logger.info(fmt::format("OK - {} v{}", itFind->layerName, itFind->specVersion));
            itLayer++;
        }
        else
        {
            logger.info(fmt::format("FAILED - {} - Removing from layers", *itLayer));
            itLayer = layers.erase(itLayer);
            allSupported = false;
        }
    }
    if (!allSupported)
    {
        logger.warning("Some layers were removed because they are not supported by the current instance");
    }
    return layers;
}

}  // namespace

VulkanExtensions::VulkanExtensions(const ILogger& logger, const VulkanGlobalFcts& rFcts, bool isDebugEnabled,
                                   const vector<string_view>& rRequiredInstanceExtensions)
  : m_debugUtilsEnabled(isDebugEnabled)
  , m_instanceExtensions(generateInstanceExtensions(logger, rFcts, m_debugUtilsEnabled, rRequiredInstanceExtensions))
  , m_deviceExtensions(generateDeviceExtensions())
  , m_layers(generateLayers(logger, rFcts, m_debugUtilsEnabled))
{
}
