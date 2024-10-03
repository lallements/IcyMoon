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
    auto getMockVmaAllocator() const -> VmaAllocator { return m_vmaAllocator; }
    auto getMockDeviceFcts() -> MockVulkanDeviceFcts& { return m_mockVulkanLoader.getMockDeviceFcts(); }
    auto getMockImageFactory() -> MockImageFactory& { return m_mockImageFactory; }

private:
    VkDevice m_vkDevice = reinterpret_cast<VkDevice>(0xbaef532f3e4a);
    VmaAllocator m_vmaAllocator = reinterpret_cast<VmaAllocator>(0xfe364b2ae3fc);
    ::testing::NiceMock<MockVulkanLoader> m_mockVulkanLoader;
    ::testing::NiceMock<MockImageFactory> m_mockImageFactory;
};

}  // namespace im3e