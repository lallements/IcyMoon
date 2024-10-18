#include <im3e/utils/types.h>

#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

TEST(TypesTest, makeVkUniquePtr)
{
    const auto mockVkDevice = reinterpret_cast<VkDevice>(0x34ef5a);
    const auto mockVkImage = reinterpret_cast<VkImage>(0x23e45d);
    MockFunction<void(VkDevice, VkImage, const VkAllocationCallbacks*)> mockDestructor;

    auto pUniquePtr = makeVkUniquePtr<VkImage>(mockVkDevice, mockVkImage, mockDestructor.AsStdFunction());

    EXPECT_CALL(mockDestructor, Call(mockVkDevice, mockVkImage, IsNull()));
    pUniquePtr.reset();
}
