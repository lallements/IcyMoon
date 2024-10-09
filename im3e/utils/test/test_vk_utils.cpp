#include <im3e/utils/vk_utils.h>

#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

TEST(VkUtilsTest, VkExtent2DEqOperator)
{
    const VkExtent2D vkExtent1{.width = 1920U, .height = 1080U};
    const VkExtent2D vkExtent2{.width = 1080U, .height = 1080U};
    const VkExtent2D vkExtent3{.width = 1920U, .height = 1920U};
    const VkExtent2D vkExtent4{.width = 3840U, .height = 2160U};
    EXPECT_THAT(vkExtent1 == vkExtent1, IsTrue());
    EXPECT_THAT(vkExtent1 == vkExtent2, IsFalse());
    EXPECT_THAT(vkExtent1 == vkExtent3, IsFalse());
    EXPECT_THAT(vkExtent1 == vkExtent4, IsFalse());
}

TEST(VkUtilsTest, VkExtent2DDiffOperator)
{
    const VkExtent2D vkExtent1{.width = 1920U, .height = 1080U};
    const VkExtent2D vkExtent2{.width = 1080U, .height = 1080U};
    const VkExtent2D vkExtent3{.width = 1920U, .height = 1920U};
    const VkExtent2D vkExtent4{.width = 3840U, .height = 2160U};
    EXPECT_THAT(vkExtent1 != vkExtent1, IsFalse());
    EXPECT_THAT(vkExtent1 != vkExtent2, IsTrue());
    EXPECT_THAT(vkExtent1 != vkExtent3, IsTrue());
    EXPECT_THAT(vkExtent1 != vkExtent4, IsTrue());
}

TEST(VkUtilsTest, getVkList)
{
    const VkDevice vkDevice = reinterpret_cast<VkDevice>(0xc0ffee);
    const char* pName = "Hello";
    const uint32_t expectedCount = 4U;
    const vector<float> expectedValues{0.0F, 1.0F, 2.0F, 3.0F};

    MockFunction<VkResult(VkDevice, const char* pName, uint32_t* pCount, float* pValues)> mockVkFct;
    {
        InSequence s;
        EXPECT_CALL(mockVkFct, Call(vkDevice, StrEq(pName), NotNull(), IsNull()))
            .WillOnce(Invoke([&](Unused, Unused, uint32_t* pCount, Unused) {
                *pCount = expectedCount;
                return VK_SUCCESS;
            }));
        EXPECT_CALL(mockVkFct, Call(vkDevice, StrEq(pName), NotNull(), NotNull()))
            .WillOnce(Invoke([&](Unused, Unused, uint32_t* pCount, float* pValues) {
                EXPECT_THAT(*pCount, Eq(expectedCount));
                ranges::copy(expectedValues, pValues);
                return VK_SUCCESS;
            }));
    }

    const auto actualValues = getVkList<float>(mockVkFct.AsStdFunction(), "mockVkFct", vkDevice, pName);
    EXPECT_THAT(actualValues, ContainerEq(expectedValues));
}

TEST(VkUtilsTest, vkFlagsContain)
{
    EXPECT_THAT(vkFlagsContain(VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT),
                IsTrue());
    EXPECT_THAT(
        vkFlagsContain(VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT),
        IsFalse());
}
