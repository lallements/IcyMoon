#include <vulkan_loader/vulkan_loader.h>

#include <gmock/gmock.h>

TEST(VulkanLoader, constructor)
{
    EXPECT_THAT(true, testing::IsFalse());
}