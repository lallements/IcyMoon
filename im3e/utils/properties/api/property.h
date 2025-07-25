#pragma once

#include <functional>
#include <memory>
#include <typeindex>
#include <vector>

namespace im3e {

class IProperty
{
public:
    virtual ~IProperty() = default;

    virtual void registerOnChange(std::weak_ptr<std::function<void()>> pOnChangeCallback) const = 0;

    virtual auto getName() const -> std::string = 0;
    virtual auto getType() const -> std::type_index = 0;

    virtual auto getChildren() const -> std::vector<std::shared_ptr<IProperty>> { return {}; }
};

}  // namespace im3e