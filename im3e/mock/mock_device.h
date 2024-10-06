#pragma once

#include "mock_image.h"
#include "mock_vulkan_loader.h"

#include <im3e/api/device.h>

#include <gmock/gmock.h>

#include <memory>

namespace im3e {

class MockDevice : public IDevice
{
public:
    MockDevice();
    ~MockDevice() override;

    MOCK_METHOD(VkDevice, getVkDevice, (), (const, override));
    MOCK_METHOD(VmaAllocator, getVmaAllocator, (), (const, override));
    MOCK_METHOD(const VulkanDeviceFcts&, getFcts, (), (const, override));
    MOCK_METHOD(std::shared_ptr<const IImageFactory>, getImageFactory, (), (const, override));

    auto createMockProxy() -> std::unique_ptr<IDevice>;

    auto getMockVkDevice() const -> VkDevice { return m_vkDevice; }
    auto getMockVmaAllocator() const -> VmaAllocator { return m_pVmaAllocator.get(); }
    auto getMockDeviceFcts() -> MockVulkanDeviceFcts& { return m_mockVulkanLoader.getMockDeviceFcts(); }
    auto getMockImageFactory() -> MockImageFactory& { return m_mockImageFactory; }

private:
    const VkInstance m_vkInstance = reinterpret_cast<VkInstance>(0x35e2ca18b3e);
    const VkPhysicalDevice m_vkPhysicalDevice = reinterpret_cast<VkPhysicalDevice>(0xef45a3c4);
    const VkDevice m_vkDevice = reinterpret_cast<VkDevice>(0xbaef532f3e4a);

    ::testing::NiceMock<MockVulkanLoader> m_mockVulkanLoader;

    VkUniquePtr<VmaAllocator> m_pVmaAllocator;
    ::testing::NiceMock<MockImageFactory> m_mockImageFactory;
};

}  // namespace im3e