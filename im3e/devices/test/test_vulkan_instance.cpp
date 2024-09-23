#include "vulkan_instance.h"

#include <im3e/mock/mock_vulkan_loader.h>

#include <gmock/gmock.h>

using namespace im3e;
using namespace std;

struct VulkanInstanceTest : public ::testing::Test
{
    auto createInstance() { return VulkanInstance(m_mockVkLoader.createMockProxy()); }

    ::testing::NiceMock<MockVulkanLoader> m_mockVkLoader;
};

TEST_F(VulkanInstanceTest, constructor)
{
    auto mockVkInstance = reinterpret_cast<VkInstance>(0xa3ef4d23b85);

    EXPECT_CALL(m_mockVkLoader, loadGlobalFcts());
    EXPECT_CALL(m_mockVkLoader.getMockGlobalFcts(),
                vkCreateInstance(::testing::NotNull(), nullptr, ::testing::NotNull()))
        .WillOnce(::testing::Invoke([&](auto* pCreateInfo, ::testing::Unused, auto* pVkInstance) {
            EXPECT_THAT(pCreateInfo->sType, ::testing::Eq(VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO));
            EXPECT_THAT(pCreateInfo->pApplicationInfo, ::testing::NotNull());

            auto pAppInfo = pCreateInfo->pApplicationInfo;
            EXPECT_THAT(pAppInfo->sType, ::testing::Eq(VK_STRUCTURE_TYPE_APPLICATION_INFO));
            EXPECT_THAT(pAppInfo->apiVersion, ::testing::Eq(VK_API_VERSION_1_3));

            *pVkInstance = mockVkInstance;
            return VK_SUCCESS;
        }));
    EXPECT_CALL(m_mockVkLoader, loadInstanceFcts(mockVkInstance));
    auto instance = createInstance();
}
