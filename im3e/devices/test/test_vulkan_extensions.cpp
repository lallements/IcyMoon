#include "src/vulkan_extensions.h"

#include <im3e/mock/mock_logger.h>
#include <im3e/mock/mock_vulkan_loader.h>
#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

struct VulkanExtensionsTest : public Test
{
    NiceMock<MockLogger> m_mockLogger;
    NiceMock<MockVulkanLoader> m_mockVk;
    VulkanGlobalFcts m_globalFcts = m_mockVk.loadGlobalFcts();
};

TEST_F(VulkanExtensionsTest, constructor)
{
    VulkanExtensions extensions(m_mockLogger, m_globalFcts, false);
    EXPECT_THAT(extensions.getInstanceExtensions(), ContainerEq(vector<const char*>{}));
    EXPECT_THAT(extensions.getDeviceExtensions(), IsSupersetOf(vector<string>{
                                                      VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                                                      VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                                                      VK_KHR_RAY_QUERY_EXTENSION_NAME,
                                                      VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                                                      VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
                                                  }));
    EXPECT_THAT(extensions.getLayers(), ContainerEq(vector<const char*>{}));
}