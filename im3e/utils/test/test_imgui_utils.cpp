#include "imgui_utils.h"

#include <im3e/test_utils/test_utils.h>

using namespace im3e;

namespace {

constexpr bool IsOpen = true;
constexpr bool IsNotOpen = false;
constexpr bool AlwaysEndScope = true;

}  // namespace

TEST(ImguiUtilsTest, ImguiScopeWhenOpen)
{
    bool callbackCalled = false;
    {
        ImguiScope scope(IsOpen, [&callbackCalled] { callbackCalled = true; });
        EXPECT_THAT(scope.isOpen(), Eq(IsOpen));
    }
    EXPECT_THAT(callbackCalled, IsTrue());
}

TEST(ImguiUtilsTest, ImguiScopeWhenNotOpen)
{
    bool callbackCalled = false;
    {
        ImguiScope scope(IsNotOpen, [&callbackCalled] { callbackCalled = true; });
        EXPECT_THAT(scope.isOpen(), Eq(IsNotOpen));
    }
    EXPECT_THAT(callbackCalled, IsFalse());
}

TEST(ImguiUtilsTest, ImGuiScopeWithAlwaysEndScopeWhenOpen)
{
    bool callbackCalled = false;
    {
        ImguiScope scope(
            IsOpen, [&callbackCalled] { callbackCalled = true; }, AlwaysEndScope);
        EXPECT_THAT(scope.isOpen(), Eq(IsOpen));
    }
    EXPECT_THAT(callbackCalled, IsTrue());
}

TEST(ImguiUtilsTest, ImGuiScopeWithAlwaysEndScopeWhenNotOpen)
{
    bool callbackCalled = false;
    {
        ImguiScope scope(
            IsNotOpen, [&callbackCalled] { callbackCalled = true; }, AlwaysEndScope);
        EXPECT_THAT(scope.isOpen(), Eq(IsNotOpen));
    }
    EXPECT_THAT(callbackCalled, IsTrue());
}
