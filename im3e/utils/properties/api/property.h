#pragma once

#include <any>
#include <functional>
#include <memory>
#include <string_view>
#include <typeindex>
#include <vector>

namespace im3e {

class IProperty
{
public:
    virtual ~IProperty() = default;

    virtual void registerOnChange(std::weak_ptr<std::function<void()>> pOnChangeCallback) const = 0;

    virtual void setAnyValue(std::any value) = 0;

    virtual auto getName() const -> std::string_view = 0;
    virtual auto getType() const -> std::type_index = 0;
    virtual auto getAnyValue() const -> std::any = 0;

    virtual auto getChildren() const -> std::vector<std::shared_ptr<IProperty>> { return {}; }
};

}  // namespace im3e