#include <im3e/vulkan_loaders/vulkan_loaders.h>

#include <gmock/gmock.h>

#include <iostream>

using namespace im3e;
using namespace std;

TEST(VulkanLoaderIntegration, constructor)
{
    auto pLoader = createVulkanLoader();
    ASSERT_THAT(pLoader, testing::NotNull());
}

TEST(VulkanLoaderIntegration, loadGlobalFcts)
{
    auto pLoader = createVulkanLoader();
    auto fcts = pLoader->loadGlobalFcts();
    ASSERT_THAT(fcts.vkEnumerateInstanceVersion, testing::NotNull());
    ASSERT_THAT(fcts.vkEnumerateInstanceExtensionProperties, testing::NotNull());
    ASSERT_THAT(fcts.vkEnumerateInstanceLayerProperties, testing::NotNull());
    ASSERT_THAT(fcts.vkCreateInstance, testing::NotNull());

    uint32_t vkVersion{};
    fcts.vkEnumerateInstanceVersion(&vkVersion);
    EXPECT_THAT(VK_API_VERSION_MAJOR(vkVersion), testing::Ge(1U));
    EXPECT_THAT(VK_API_VERSION_MINOR(vkVersion), testing::Ge(3U));
}