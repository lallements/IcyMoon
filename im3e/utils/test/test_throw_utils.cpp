#include <im3e/utils/throw_utils.h>

#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

TEST(ThrowUtilsTest, throwIfNullThrowsWhenNull)
{
    const void* pNullPtr = nullptr;
    EXPECT_THROW(throwIfNull<runtime_error>(pNullPtr, "Expected Throw"), runtime_error);
}

TEST(ThrowUtilsTest, throwIfNullDoesNotThrowWhenNotNull)
{
    const void* pNonNullPtr = reinterpret_cast<const void*>(0x1234);
    EXPECT_NO_THROW(throwIfNull<runtime_error>(pNonNullPtr, "Unexpected Throw"));
}

TEST(ThrowUtilsTest, throwIfArgNullThrowsWhenNull)
{
    const void* pNullPtr = nullptr;
    EXPECT_THROW(throwIfArgNull(pNullPtr, "Expected Throw"), invalid_argument);
}

TEST(ThrowUtilsTest, throwIfArgNullDoesNotThrowWhenNotNull)
{
    const void* pNonNullPtr = reinterpret_cast<const void*>(0x1234);
    EXPECT_NO_THROW(throwIfArgNull(pNonNullPtr, "Unexpected Throw"));
}

TEST(ThrowUtilsTest, throwIfVkFailedThrowsOnFailure)
{
    EXPECT_THROW(throwIfVkFailed(VK_ERROR_UNKNOWN, "Expected Failure"), runtime_error);
}

TEST(ThrowUtilsTest, throwIfVkFailedDoesNotThrowOnSuccess)
{
    EXPECT_NO_THROW(throwIfVkFailed(VK_SUCCESS, "Unexpected Failure"));
}