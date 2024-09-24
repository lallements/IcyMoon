#include "vulkan_instance.h"

#include <im3e/mock/mock_logger.h>
#include <im3e/mock/mock_vulkan_loader.h>
#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

struct VulkanInstanceTest : public Test
{
    auto createInstance(bool isDebugEnabled = false)
    {
        return VulkanInstance(m_mockLogger, isDebugEnabled, m_mockVkLoader.createMockProxy());
    }

    NiceMock<MockLogger> m_mockLogger;
    NiceMock<MockVulkanLoader> m_mockVkLoader;
};

TEST_F(VulkanInstanceTest, constructor)
{
    auto mockVkInstance = reinterpret_cast<VkInstance>(0xa3ef4d23b85);

    EXPECT_CALL(m_mockVkLoader, loadGlobalFcts());
    EXPECT_CALL(m_mockVkLoader.getMockGlobalFcts(), vkCreateInstance(NotNull(), nullptr, NotNull()))
        .WillOnce(Invoke([&](auto* pCreateInfo, Unused, auto* pVkInstance) {
            EXPECT_THAT(pCreateInfo->sType, Eq(VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO));
            EXPECT_THAT(pCreateInfo->pApplicationInfo, NotNull());

            auto pAppInfo = pCreateInfo->pApplicationInfo;
            EXPECT_THAT(pAppInfo->sType, Eq(VK_STRUCTURE_TYPE_APPLICATION_INFO));
            EXPECT_THAT(pAppInfo->apiVersion, Eq(VK_API_VERSION_1_3));

            *pVkInstance = mockVkInstance;
            return VK_SUCCESS;
        }));
    EXPECT_CALL(m_mockVkLoader, loadInstanceFcts(mockVkInstance));
    auto instance = createInstance();
}
