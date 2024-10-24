#pragma once

#include "mock_command_buffer.h"
#include "mock_image.h"
#include "mock_logger.h"
#include "mock_vulkan_loader.h"

#include <im3e/api/device.h>

#include <gmock/gmock.h>

#include <memory>

namespace im3e {

class MockMemoryAllocator : public IMemoryAllocator
{
public:
    MockMemoryAllocator();
    ~MockMemoryAllocator() override;

    MOCK_METHOD(VkResult, createImage,
                (const VkImageCreateInfo* pVkCreateInfo, const VmaAllocationCreateInfo* pVmaCreateInfo,
                 VkImage* pVkImage, VmaAllocation* pVmaAllocation, VmaAllocationInfo* pVmaAllocationInfo),
                (override));
    MOCK_METHOD(void, destroyImage, (VkImage vkImage, VmaAllocation vmaAllocation), (override));

    MOCK_METHOD(VkResult, mapMemory, (VmaAllocation vmaAllocation, void** ppData), (override));
    MOCK_METHOD(void, unmapMemory, (VmaAllocation vmaAllocation), (override));

    auto createMockProxy() -> std::unique_ptr<IMemoryAllocator>;
};

class MockDevice : public IDevice
{
public:
    MockDevice();
    ~MockDevice() override;

    MOCK_METHOD(std::unique_ptr<ILogger>, createLogger, (std::string_view name), (const, override));

    MOCK_METHOD(VkInstance, getVkInstance, (), (const, override));
    MOCK_METHOD(VkPhysicalDevice, getVkPhysicalDevice, (), (const, override));
    MOCK_METHOD(VkDevice, getVkDevice, (), (const, override));
    MOCK_METHOD(const VulkanDeviceFcts&, getFcts, (), (const, override));
    MOCK_METHOD(std::shared_ptr<IMemoryAllocator>, getMemoryAllocator, (), (const, override));
    MOCK_METHOD(std::shared_ptr<const IImageFactory>, getImageFactory, (), (const, override));
    MOCK_METHOD(std::shared_ptr<ICommandQueue>, getCommandQueue, (), (override));

    auto createMockProxy() -> std::unique_ptr<IDevice>;

    auto getMockLogger() -> MockLogger& { return m_mockLogger; }
    auto getMockVkInstance() const -> VkInstance { return m_vkInstance; }
    auto getMockVkPhysicalDevice() const -> VkPhysicalDevice { return m_vkPhysicalDevice; }
    auto getMockVkDevice() const -> VkDevice { return m_vkDevice; }
    auto getMockVulkanLoader() -> MockVulkanLoader& { return m_mockVulkanLoader; }
    auto getMockDeviceFcts() -> MockVulkanDeviceFcts& { return m_mockVulkanLoader.getMockDeviceFcts(); }
    auto getMockMemoryAllocator() -> MockMemoryAllocator& { return m_mockMemoryAllocator; }
    auto getMockImageFactory() -> MockImageFactory& { return m_mockImageFactory; }
    auto getMockCommandQueue() -> MockCommandQueue& { return m_mockCommandQueue; }

private:
    const VkInstance m_vkInstance = reinterpret_cast<VkInstance>(0x35e2ca18b3e);
    const VkPhysicalDevice m_vkPhysicalDevice = reinterpret_cast<VkPhysicalDevice>(0xef45a3c4);
    const VkDevice m_vkDevice = reinterpret_cast<VkDevice>(0xbaef532f3e4a);

    ::testing::NiceMock<MockLogger> m_mockLogger;
    ::testing::NiceMock<MockVulkanLoader> m_mockVulkanLoader;
    ::testing::NiceMock<MockMemoryAllocator> m_mockMemoryAllocator;
    ::testing::NiceMock<MockImageFactory> m_mockImageFactory;
    ::testing::NiceMock<MockCommandQueue> m_mockCommandQueue;
};

}  // namespace im3e