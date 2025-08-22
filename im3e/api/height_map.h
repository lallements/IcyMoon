#pragma once

#include <string>

namespace im3e {

class IHeightMap
{
public:
    virtual ~IHeightMap() = default;

    virtual auto getName() const -> std::string = 0;
};

}  // namespace im3e