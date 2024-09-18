#include "src/vulkan_loader.h"

#include <im3e/utils/throw_utils.h>

#include <gmock/gmock.h>

using namespace im3e;
using namespace std;

namespace {

class VulkanLoaderTest;
VulkanLoaderTest* g_pTestFixture = nullptr;

PFN_vkVoidFunction mockVkGetInstanceProcAddr(VkInstance vkInstance, const char* name);
PFN_vkVoidFunction mockVkGetDeviceProcAddr(VkDevice vkDevice, const char* name);

struct VulkanLoaderTest : public ::testing::Test
{
    VulkanLoaderTest()
    {
        throwIfFalse<logic_error>(g_pTestFixture == nullptr, "Cannot have more than one VulkanLoaderTest at a time");
        g_pTestFixture = this;
    }
    ~VulkanLoaderTest() override { g_pTestFixture = nullptr; }

    auto createMockLibrary()
    {
        return UniquePtrWithDeleter<void>(reinterpret_cast<void*>(0x1234), [](auto*) {});
    }

    auto createLoader()
    {
        return make_unique<VulkanLoader>(createMockLibrary(), &mockVkGetInstanceProcAddr, &mockVkGetDeviceProcAddr);
    }

    ::testing::MockFunction<PFN_vkVoidFunction(VkInstance, const char*)> m_mockVkGetInstanceProcAddr;
    ::testing::MockFunction<PFN_vkVoidFunction(VkDevice, const char*)> m_mockVkGetDeviceProcAddr;
};

PFN_vkVoidFunction mockVkGetInstanceProcAddr(VkInstance vkInstance, const char* name)
{
    throwIfNull<logic_error>(g_pTestFixture, "Cannot test VulkanLoader without test fixture");
    return g_pTestFixture->m_mockVkGetInstanceProcAddr.Call(vkInstance, name);
}

PFN_vkVoidFunction mockVkGetDeviceProcAddr(VkDevice vkDevice, const char* name)
{
    throwIfNull<logic_error>(g_pTestFixture, "Cannot test VulkanLoader without test fixture");
    return g_pTestFixture->m_mockVkGetDeviceProcAddr.Call(vkDevice, name);
}

}  // namespace

TEST_F(VulkanLoaderTest, constructor)
{
    auto pLoader = createLoader();
    ASSERT_THAT(pLoader, ::testing::NotNull());
}

TEST_F(VulkanLoaderTest, constructorThrowsWithoutLibrary)
{
    EXPECT_THROW(VulkanLoader loader(nullptr, &mockVkGetInstanceProcAddr, &mockVkGetDeviceProcAddr), invalid_argument);
}

TEST_F(VulkanLoaderTest, constructorThrowsWithoutVkGetInstanceProcAddr)
{
    EXPECT_THROW(VulkanLoader loader(createMockLibrary(), nullptr, &mockVkGetDeviceProcAddr), invalid_argument);
}

TEST_F(VulkanLoaderTest, constructorThrowsWithoutVkGetDeviceProcAddr)
{
    EXPECT_THROW(VulkanLoader loader(createMockLibrary(), &mockVkGetInstanceProcAddr, nullptr), invalid_argument);
}

TEST_F(VulkanLoaderTest, loadGlobalFcts)
{
    auto pLoader = createLoader();

    auto expectGlobalFctLoaded = [&](string_view name) {
        EXPECT_CALL(m_mockVkGetInstanceProcAddr, Call(::testing::IsNull(), ::testing::StrEq(name)))
            .WillOnce(::testing::Return(reinterpret_cast<PFN_vkVoidFunction>(0x1234)));
    };
    expectGlobalFctLoaded("vkEnumerateInstanceVersion");
    expectGlobalFctLoaded("vkEnumerateInstanceExtensionProperties");
    expectGlobalFctLoaded("vkEnumerateInstanceLayerProperties");
    expectGlobalFctLoaded("vkCreateInstance");

    auto globalFcts = pLoader->loadGlobalFcts();
    EXPECT_THAT(globalFcts.vkEnumerateInstanceVersion, ::testing::NotNull());
    EXPECT_THAT(globalFcts.vkEnumerateInstanceExtensionProperties, ::testing::NotNull());
    EXPECT_THAT(globalFcts.vkEnumerateInstanceLayerProperties, ::testing::NotNull());
    EXPECT_THAT(globalFcts.vkCreateInstance, ::testing::NotNull());
}

TEST_F(VulkanLoaderTest, loadInstanceFcts)
{
    auto pLoader = createLoader();

    auto mockVkInstance = reinterpret_cast<VkInstance>(0x34ef3a);

    auto expectInstFctLoaded = [&](string_view name) {
        EXPECT_CALL(m_mockVkGetInstanceProcAddr, Call(mockVkInstance, ::testing::StrEq(name)))
            .WillOnce(::testing::Return(reinterpret_cast<PFN_vkVoidFunction>(0x4567)));
    };
    expectInstFctLoaded("vkDestroyInstance");
    expectInstFctLoaded("vkCreateDevice");

    auto instanceFcts = pLoader->loadInstanceFcts(mockVkInstance);
    EXPECT_THAT(instanceFcts.vkDestroyInstance, ::testing::NotNull());
    EXPECT_THAT(instanceFcts.vkCreateDevice, ::testing::NotNull());
}

TEST_F(VulkanLoaderTest, loadInstanceFctsThrowsWithoutInstance)
{
    auto pLoader = createLoader();
    EXPECT_THROW(pLoader->loadInstanceFcts(nullptr), invalid_argument);
}

TEST_F(VulkanLoaderTest, loadDeviceFcts)
{
    auto pLoader = createLoader();

    auto mockVkDevice = reinterpret_cast<VkDevice>(0xaf34d2ce8);

    auto expectDeviceFctLoaded = [&](string_view name) {
        EXPECT_CALL(m_mockVkGetDeviceProcAddr, Call(mockVkDevice, ::testing::StrEq(name)))
            .WillOnce(::testing::Return(reinterpret_cast<PFN_vkVoidFunction>(0xabcde)));
    };
    expectDeviceFctLoaded("vkDestroyDevice");

    auto deviceFcts = pLoader->loadDeviceFcts(mockVkDevice);
    EXPECT_THAT(deviceFcts.vkDestroyDevice, ::testing::NotNull());
}

TEST_F(VulkanLoaderTest, loadDeviceFctsThrowsWithoutDevice)
{
    auto pLoader = createLoader();
    EXPECT_THROW(pLoader->loadDeviceFcts(nullptr), invalid_argument);
}