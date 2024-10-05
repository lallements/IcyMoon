#include "src/vulkan_loader.h"

#include <im3e/test_utils/test_utils.h>
#include <im3e/utils/throw_utils.h>

using namespace im3e;
using namespace std;

namespace {

class VulkanLoaderTest;
VulkanLoaderTest* g_pTestFixture = nullptr;

PFN_vkVoidFunction mockVkGetInstanceProcAddr(VkInstance vkInstance, const char* name);
PFN_vkVoidFunction mockVkGetDeviceProcAddr(VkDevice vkDevice, const char* name);

struct VulkanLoaderTest : public Test
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
        return make_unique<VulkanLoader>(
            VulkanLoaderConfig{
                .isDebugEnabled = true,
            },
            createMockLibrary(), &mockVkGetInstanceProcAddr, &mockVkGetDeviceProcAddr);
    }

    void expectGlobalFctLoaded(string_view name)
    {
        EXPECT_CALL(m_mockVkGetInstanceProcAddr, Call(IsNull(), StrEq(name)))
            .WillOnce(Return(reinterpret_cast<PFN_vkVoidFunction>(0x1234)));
    }

    void expectInstFctLoaded(VkInstance vkInstance, string_view name)
    {
        EXPECT_CALL(m_mockVkGetInstanceProcAddr, Call(vkInstance, StrEq(name)))
            .WillOnce(Return(reinterpret_cast<PFN_vkVoidFunction>(0x4567)));
    }

    void expectDeviceFctLoaded(VkDevice vkDevice, string_view name)
    {
        EXPECT_CALL(m_mockVkGetDeviceProcAddr, Call(vkDevice, StrEq(name)))
            .WillOnce(Return(reinterpret_cast<PFN_vkVoidFunction>(0xabcde)));
    }

    MockFunction<PFN_vkVoidFunction(VkInstance, const char*)> m_mockVkGetInstanceProcAddr;
    MockFunction<PFN_vkVoidFunction(VkDevice, const char*)> m_mockVkGetDeviceProcAddr;
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

#if defined(__linux__)
TEST_F(VulkanLoaderTest, createVulkanLoader)
{
    auto pLoader = createVulkanLoader(VulkanLoaderConfig{});
    ASSERT_THAT(pLoader, NotNull());
}
#endif

TEST_F(VulkanLoaderTest, constructor)
{
    auto pLoader = createLoader();
    ASSERT_THAT(pLoader, NotNull());
}

TEST_F(VulkanLoaderTest, constructorThrowsWithoutLibrary)
{
    EXPECT_THROW(
        VulkanLoader loader(VulkanLoaderConfig{}, nullptr, &mockVkGetInstanceProcAddr, &mockVkGetDeviceProcAddr),
        invalid_argument);
}

TEST_F(VulkanLoaderTest, constructorThrowsWithoutVkGetInstanceProcAddr)
{
    EXPECT_THROW(VulkanLoader loader(VulkanLoaderConfig{}, createMockLibrary(), nullptr, &mockVkGetDeviceProcAddr),
                 invalid_argument);
}

TEST_F(VulkanLoaderTest, constructorThrowsWithoutVkGetDeviceProcAddr)
{
    EXPECT_THROW(VulkanLoader loader(VulkanLoaderConfig{}, createMockLibrary(), &mockVkGetInstanceProcAddr, nullptr),
                 invalid_argument);
}

TEST_F(VulkanLoaderTest, loadGlobalFcts)
{
    auto pLoader = createLoader();

    expectGlobalFctLoaded("vkEnumerateInstanceVersion");
    expectGlobalFctLoaded("vkEnumerateInstanceExtensionProperties");
    expectGlobalFctLoaded("vkEnumerateInstanceLayerProperties");
    expectGlobalFctLoaded("vkCreateInstance");
    auto globalFcts = pLoader->loadGlobalFcts();
    EXPECT_THAT(globalFcts.vkEnumerateInstanceVersion, NotNull());
    EXPECT_THAT(globalFcts.vkEnumerateInstanceExtensionProperties, NotNull());
    EXPECT_THAT(globalFcts.vkEnumerateInstanceLayerProperties, NotNull());
    EXPECT_THAT(globalFcts.vkCreateInstance, NotNull());
}

TEST_F(VulkanLoaderTest, loadInstanceFcts)
{
    auto pLoader = createLoader();
    const auto vkInstance = reinterpret_cast<VkInstance>(0x34ef3a);

    expectInstFctLoaded(vkInstance, "vkDestroyInstance");
    expectInstFctLoaded(vkInstance, "vkCreateDevice");
    expectInstFctLoaded(vkInstance, "vkCreateDebugUtilsMessengerEXT");
    expectInstFctLoaded(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
    expectInstFctLoaded(vkInstance, "vkEnumeratePhysicalDevices");
    expectInstFctLoaded(vkInstance, "vkGetPhysicalDeviceProperties");
    expectInstFctLoaded(vkInstance, "vkGetPhysicalDeviceFeatures");
    expectInstFctLoaded(vkInstance, "vkEnumerateDeviceExtensionProperties");
    expectInstFctLoaded(vkInstance, "vkGetPhysicalDeviceQueueFamilyProperties");
    expectInstFctLoaded(vkInstance, "vkGetPhysicalDeviceMemoryProperties");
    auto instanceFcts = pLoader->loadInstanceFcts(vkInstance);
    EXPECT_THAT(instanceFcts.vkDestroyInstance, NotNull());
    EXPECT_THAT(instanceFcts.vkCreateDevice, NotNull());
}

TEST_F(VulkanLoaderTest, loadInstanceFctsThrowsWithoutInstance)
{
    auto pLoader = createLoader();
    EXPECT_THROW(pLoader->loadInstanceFcts(nullptr), invalid_argument);
}

TEST_F(VulkanLoaderTest, loadDeviceFcts)
{
    auto pLoader = createLoader();
    const auto vkDevice = reinterpret_cast<VkDevice>(0xaf34d2ce8);

    expectDeviceFctLoaded(vkDevice, "vkDestroyDevice");
    expectDeviceFctLoaded(vkDevice, "vkGetDeviceQueue");
    expectDeviceFctLoaded(vkDevice, "vkGetImageSubresourceLayout");
    expectDeviceFctLoaded(vkDevice, "vkMapMemory");
    expectDeviceFctLoaded(vkDevice, "vkUnmapMemory");
    auto deviceFcts = pLoader->loadDeviceFcts(vkDevice);
    EXPECT_THAT(deviceFcts.vkDestroyDevice, NotNull());
}

TEST_F(VulkanLoaderTest, loadDeviceFctsThrowsWithoutDevice)
{
    auto pLoader = createLoader();
    EXPECT_THROW(pLoader->loadDeviceFcts(nullptr), invalid_argument);
}

TEST_F(VulkanLoaderTest, loadVmaFcts)
{
    auto pLoader = createLoader();
    const auto vkInstance = reinterpret_cast<VkInstance>(0xf00d);
    const auto vkDevice = reinterpret_cast<VkDevice>(0x8e4f3c);

    expectInstFctLoaded(vkInstance, "vkGetPhysicalDeviceProperties");
    expectInstFctLoaded(vkInstance, "vkGetPhysicalDeviceMemoryProperties");
    expectDeviceFctLoaded(vkDevice, "vkAllocateMemory");
    expectDeviceFctLoaded(vkDevice, "vkFreeMemory");
    expectDeviceFctLoaded(vkDevice, "vkMapMemory");
    expectDeviceFctLoaded(vkDevice, "vkUnmapMemory");
    expectDeviceFctLoaded(vkDevice, "vkFlushMappedMemoryRanges");
    expectDeviceFctLoaded(vkDevice, "vkInvalidateMappedMemoryRanges");
    expectDeviceFctLoaded(vkDevice, "vkBindBufferMemory");
    expectDeviceFctLoaded(vkDevice, "vkBindImageMemory");
    expectDeviceFctLoaded(vkDevice, "vkGetBufferMemoryRequirements");
    expectDeviceFctLoaded(vkDevice, "vkGetImageMemoryRequirements");
    expectDeviceFctLoaded(vkDevice, "vkCreateBuffer");
    expectDeviceFctLoaded(vkDevice, "vkDestroyBuffer");
    expectDeviceFctLoaded(vkDevice, "vkCreateImage");
    expectDeviceFctLoaded(vkDevice, "vkDestroyImage");
    expectDeviceFctLoaded(vkDevice, "vkCmdCopyBuffer");
    expectDeviceFctLoaded(vkDevice, "vkGetBufferMemoryRequirements2");
    expectDeviceFctLoaded(vkDevice, "vkGetImageMemoryRequirements2");
    expectDeviceFctLoaded(vkDevice, "vkBindBufferMemory2");
    expectDeviceFctLoaded(vkDevice, "vkBindImageMemory2");
    expectInstFctLoaded(vkInstance, "vkGetPhysicalDeviceMemoryProperties2");
    expectDeviceFctLoaded(vkDevice, "vkGetDeviceBufferMemoryRequirements");
    expectDeviceFctLoaded(vkDevice, "vkGetDeviceImageMemoryRequirements");
    auto vmaFcts = pLoader->loadVmaFcts(vkInstance, vkDevice);
    EXPECT_THAT(vmaFcts.vkGetPhysicalDeviceProperties, NotNull());
    EXPECT_THAT(vmaFcts.vkGetPhysicalDeviceMemoryProperties, NotNull());
    EXPECT_THAT(vmaFcts.vkAllocateMemory, NotNull());
    EXPECT_THAT(vmaFcts.vkFreeMemory, NotNull());
    EXPECT_THAT(vmaFcts.vkMapMemory, NotNull());
    EXPECT_THAT(vmaFcts.vkUnmapMemory, NotNull());
    EXPECT_THAT(vmaFcts.vkFlushMappedMemoryRanges, NotNull());
    EXPECT_THAT(vmaFcts.vkInvalidateMappedMemoryRanges, NotNull());
    EXPECT_THAT(vmaFcts.vkBindBufferMemory, NotNull());
    EXPECT_THAT(vmaFcts.vkBindImageMemory, NotNull());
    EXPECT_THAT(vmaFcts.vkGetBufferMemoryRequirements, NotNull());
    EXPECT_THAT(vmaFcts.vkGetImageMemoryRequirements, NotNull());
    EXPECT_THAT(vmaFcts.vkCreateBuffer, NotNull());
    EXPECT_THAT(vmaFcts.vkDestroyBuffer, NotNull());
    EXPECT_THAT(vmaFcts.vkCreateImage, NotNull());
    EXPECT_THAT(vmaFcts.vkDestroyImage, NotNull());
    EXPECT_THAT(vmaFcts.vkCmdCopyBuffer, NotNull());
    EXPECT_THAT(vmaFcts.vkGetBufferMemoryRequirements2KHR, NotNull());
    EXPECT_THAT(vmaFcts.vkGetImageMemoryRequirements2KHR, NotNull());
    EXPECT_THAT(vmaFcts.vkBindBufferMemory2KHR, NotNull());
    EXPECT_THAT(vmaFcts.vkBindImageMemory2KHR, NotNull());
    EXPECT_THAT(vmaFcts.vkGetPhysicalDeviceMemoryProperties2KHR, NotNull());
    EXPECT_THAT(vmaFcts.vkGetDeviceBufferMemoryRequirements, NotNull());
    EXPECT_THAT(vmaFcts.vkGetDeviceImageMemoryRequirements, NotNull());
}

TEST_F(VulkanLoaderTest, loadVmaFctsThrowsWithoutInstance)
{
    const auto mockVkDevice = reinterpret_cast<VkDevice>(0xabc);

    auto pLoader = createLoader();
    EXPECT_THROW(pLoader->loadVmaFcts(nullptr, mockVkDevice), invalid_argument);
}

TEST_F(VulkanLoaderTest, loadVmaFctsThrowsWithoutDevice)
{
    const auto mockVkInstance = reinterpret_cast<VkInstance>(0xa1b2c3);

    auto pLoader = createLoader();
    EXPECT_THROW(pLoader->loadVmaFcts(mockVkInstance, nullptr), invalid_argument);
}