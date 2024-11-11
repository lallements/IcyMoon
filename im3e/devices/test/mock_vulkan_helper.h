#pragma once

#include <im3e/mock/mock_vulkan_loader.h>
#include <im3e/test_utils/test_utils.h>

namespace im3e {

inline void expectInstanceExtensionsEnumerated(MockVulkanLoader& rMockVkLoader, bool isDebugEnabled)
{
    auto& rMockFcts = rMockVkLoader.getMockGlobalFcts();
    std::vector<VkExtensionProperties> extensions{VkExtensionProperties{
        .extensionName = VK_KHR_SURFACE_EXTENSION_NAME,
        .specVersion = 1U,
    }};
    if (isDebugEnabled)
    {
        extensions.emplace_back(VkExtensionProperties{
            .extensionName = VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            .specVersion = 1U,
        });
    }

    InSequence s;
    EXPECT_CALL(rMockFcts, vkEnumerateInstanceExtensionProperties(IsNull(), NotNull(), IsNull()))
        .WillOnce(DoAll(SetArgPointee<1>(extensions.size()), Return(VK_SUCCESS)));
    EXPECT_CALL(rMockFcts, vkEnumerateInstanceExtensionProperties(IsNull(), NotNull(), NotNull()))
        .WillOnce(Invoke([extensions](Unused, auto* pPropertyCount, auto* pProperties) {
            *pPropertyCount = static_cast<uint32_t>(extensions.size());
            std::ranges::copy(extensions, pProperties);
            return VK_SUCCESS;
        }));
}

}  // namespace im3e