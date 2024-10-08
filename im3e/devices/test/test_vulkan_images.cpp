#include "src/vulkan_images.h"

#include <im3e/mock/mock_device.h>
#include <im3e/test_utils/test_utils.h>
#include <im3e/test_utils/vk.h>

using namespace im3e;
using namespace std;

struct ImageFactoryTest : public Test
{
    auto createFactory() { return createVulkanImageFactory(m_mockDevice.createMockProxy()); }

    NiceMock<MockDevice> m_mockDevice;
    MockMemoryAllocator& m_rMockAllocator = m_mockDevice.getMockMemoryAllocator();
};

TEST_F(ImageFactoryTest, createVulkanImageFactoryThrowsIfDeviceNull)
{
    EXPECT_THROW(auto pFactory = createVulkanImageFactory(nullptr), invalid_argument);
}

TEST_F(ImageFactoryTest, createVulkanImageFactory)
{
    auto pFactory = createFactory();
    ASSERT_THAT(pFactory, NotNull());
}

TEST_F(ImageFactoryTest, createImage)
{
    auto pFactory = createFactory();

    const ImageConfig imageConfig{
        .name = "testImage",
        .vkExtent{.width = 1920U, .height = 1080U},
        .vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT,
        .vkUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .vkCreateFlags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
    };
    const auto vkImage = reinterpret_cast<VkImage>(0xab34ef56);
    const auto vmaAllocation = reinterpret_cast<VmaAllocation>(0x542e8ce);

    EXPECT_CALL(m_rMockAllocator, createImage(NotNull(), NotNull(), NotNull(), NotNull(), NotNull()))
        .WillOnce(Invoke([&](const VkImageCreateInfo* pVkCreateInfo, const VmaAllocationCreateInfo* pVmaCreateInfo,
                             VkImage* pVkImage, VmaAllocation* pVmaAllocation, VmaAllocationInfo*) {
            EXPECT_THAT(pVkCreateInfo->sType, Eq(VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO));
            EXPECT_THAT(pVkCreateInfo->flags, Eq(imageConfig.vkCreateFlags));
            EXPECT_THAT(pVkCreateInfo->imageType, Eq(VK_IMAGE_TYPE_2D));
            EXPECT_THAT(pVkCreateInfo->format, Eq(imageConfig.vkFormat));
            EXPECT_THAT(pVkCreateInfo->extent.width, Eq(imageConfig.vkExtent.width));
            EXPECT_THAT(pVkCreateInfo->extent.height, Eq(imageConfig.vkExtent.height));
            EXPECT_THAT(pVkCreateInfo->extent.depth, Eq(1U));
            EXPECT_THAT(pVkCreateInfo->mipLevels, Eq(1U));
            EXPECT_THAT(pVkCreateInfo->arrayLayers, Eq(1U));
            EXPECT_THAT(pVkCreateInfo->samples, Eq(VK_SAMPLE_COUNT_1_BIT));
            EXPECT_THAT(pVkCreateInfo->tiling, Eq(VK_IMAGE_TILING_OPTIMAL));
            EXPECT_THAT(pVkCreateInfo->usage, Eq(imageConfig.vkUsage));

            EXPECT_THAT(pVmaCreateInfo->flags, Eq(VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT));
            EXPECT_THAT(pVmaCreateInfo->usage, Eq(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE));
            EXPECT_THAT(reinterpret_cast<const char*>(pVmaCreateInfo->pUserData), StrEq(imageConfig.name));

            *pVkImage = vkImage;
            *pVmaAllocation = vmaAllocation;
            return VK_SUCCESS;
        }));

    auto pImage = pFactory->createImage(imageConfig);
    ASSERT_THAT(pImage, NotNull());
    EXPECT_THAT(pImage->getVkImage(), Eq(vkImage));
    EXPECT_THAT(pImage->getVkExtent(), Eq(imageConfig.vkExtent));
    EXPECT_THAT(pImage->getFormat(), Eq(imageConfig.vkFormat));

    EXPECT_CALL(m_rMockAllocator, destroyImage(Eq(vkImage), Eq(vmaAllocation)));
}

TEST_F(ImageFactoryTest, createHostVisibleImage)
{
    auto pFactory = createFactory();

    const ImageConfig imageConfig{
        .name = "testImage",
        .vkExtent{.width = 3840U, .height = 2160U},
        .vkFormat = VK_FORMAT_R8G8B8_SNORM,
        .vkUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .vkCreateFlags = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT,
    };
    const auto vkImage = reinterpret_cast<VkImage>(0x28e45cad);
    const auto vmaAllocation = reinterpret_cast<VmaAllocation>(0xd1a2e45f);
    const auto vkDeviceMemory = reinterpret_cast<VkDeviceMemory>(0x45efac3);
    const VkDeviceSize memOffset = 123U;
    const VkDeviceSize memSize = 234U;
    const VkDeviceSize rowPitch = 32U;

    EXPECT_CALL(m_rMockAllocator, createImage(NotNull(), NotNull(), NotNull(), NotNull(), NotNull()))
        .WillOnce(Invoke([&](const VkImageCreateInfo* pVkCreateInfo, const VmaAllocationCreateInfo* pVmaCreateInfo,
                             VkImage* pVkImage, VmaAllocation* pVmaAllocation, VmaAllocationInfo* pVmaAllocationInfo) {
            EXPECT_THAT(pVkCreateInfo->sType, Eq(VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO));
            EXPECT_THAT(pVkCreateInfo->flags, Eq(imageConfig.vkCreateFlags));
            EXPECT_THAT(pVkCreateInfo->imageType, Eq(VK_IMAGE_TYPE_2D));
            EXPECT_THAT(pVkCreateInfo->format, Eq(imageConfig.vkFormat));
            EXPECT_THAT(pVkCreateInfo->extent.width, Eq(imageConfig.vkExtent.width));
            EXPECT_THAT(pVkCreateInfo->extent.height, Eq(imageConfig.vkExtent.height));
            EXPECT_THAT(pVkCreateInfo->extent.depth, Eq(1U));
            EXPECT_THAT(pVkCreateInfo->mipLevels, Eq(1U));
            EXPECT_THAT(pVkCreateInfo->arrayLayers, Eq(1U));
            EXPECT_THAT(pVkCreateInfo->samples, Eq(VK_SAMPLE_COUNT_1_BIT));
            EXPECT_THAT(pVkCreateInfo->tiling, Eq(VK_IMAGE_TILING_LINEAR));
            EXPECT_THAT(pVkCreateInfo->usage, Eq(imageConfig.vkUsage));

            EXPECT_THAT(pVmaCreateInfo->flags, Eq(VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT));
            EXPECT_THAT(pVmaCreateInfo->usage, Eq(VMA_MEMORY_USAGE_AUTO_PREFER_HOST));
            EXPECT_THAT(reinterpret_cast<const char*>(pVmaCreateInfo->pUserData), StrEq(imageConfig.name));

            *pVkImage = vkImage;
            *pVmaAllocation = vmaAllocation;
            pVmaAllocationInfo->deviceMemory = vkDeviceMemory;
            pVmaAllocationInfo->offset = memOffset;
            pVmaAllocationInfo->size = memSize;
            return VK_SUCCESS;
        }));
    EXPECT_CALL(m_mockDevice.getMockDeviceFcts(),
                vkGetImageSubresourceLayout(m_mockDevice.getMockVkDevice(), vkImage, NotNull(), NotNull()))
        .WillOnce(Invoke([&](Unused, Unused, const VkImageSubresource* pVkSubresource, VkSubresourceLayout* pVkLayout) {
            EXPECT_THAT(pVkSubresource->aspectMask, Eq(VK_IMAGE_ASPECT_COLOR_BIT));
            pVkLayout->rowPitch = rowPitch;
        }));

    auto pImage = pFactory->createHostVisibleImage(imageConfig);
    ASSERT_THAT(pImage, NotNull());
    EXPECT_THAT(pImage->getVkImage(), Eq(vkImage));
    EXPECT_THAT(pImage->getVkExtent(), Eq(imageConfig.vkExtent));
    EXPECT_THAT(pImage->getFormat(), Eq(imageConfig.vkFormat));

    auto* pData = reinterpret_cast<uint8_t*>(0xb43e2a67cf);
    EXPECT_CALL(m_mockDevice.getMockDeviceFcts(),
                vkMapMemory(m_mockDevice.getMockVkDevice(), vkDeviceMemory, memOffset, memSize, 0U, NotNull()))
        .WillOnce(Invoke([&](Unused, Unused, Unused, Unused, Unused, void** ppData) {
            *ppData = pData;
            return VK_SUCCESS;
        }));
    auto pMapping = pImage->map();

    ASSERT_THAT(pMapping, NotNull());
    EXPECT_THAT(pMapping->getData(), Eq(pData));
    EXPECT_THAT(pMapping->getConstData(), Eq(pData));
    EXPECT_THAT(pMapping->getSizeInBytes(), Eq(memSize));
    EXPECT_THAT(pMapping->getRowPitch(), Eq(rowPitch));

    EXPECT_CALL(m_mockDevice.getMockDeviceFcts(), vkUnmapMemory(m_mockDevice.getMockVkDevice(), vkDeviceMemory));
    pMapping.reset();

    EXPECT_CALL(m_rMockAllocator, destroyImage(Eq(vkImage), Eq(vmaAllocation)));
}