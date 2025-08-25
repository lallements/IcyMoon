#pragma once

#include <glm/glm.hpp>

#include <string>

namespace im3e {

class IHeightMap
{
public:
    virtual ~IHeightMap() = default;

    virtual auto getName() const -> std::string = 0;
    virtual auto getTileSize() const -> glm::u32vec2 = 0;
};

}  // namespace im3e