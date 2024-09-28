#pragma once

#include <gmock/gmock.h>
#include <vulkan/vulkan.h>

void PrintTo(const VkExtent2D& rVkExtent, std::ostream* pStream);
void PrintTo(const VkViewport& rVkViewport, std::ostream* pStream);
