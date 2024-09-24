#include "vk.h"

#include <fmt/format.h>

void PrintTo(const VkExtent2D& rVkExtent, std::ostream* pStream)
{
    *pStream << fmt::format("(width: {}; height: {})", rVkExtent.width, rVkExtent.height);
}

void PrintTo(const VkViewport& rVkViewport, std::ostream* pStream)
{
    *pStream << fmt::format("(x: {}; y: {}; width: {}; height: {}; minDepth: {}; maxDepth: {})", rVkViewport.x,
                            rVkViewport.y, rVkViewport.width, rVkViewport.height, rVkViewport.minDepth,
                            rVkViewport.maxDepth);
}
