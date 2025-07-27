#pragma once

#include "property_change_notifier.h"

#include <im3e/utils/properties/api/property.h>

#include <optional>
#include <type_traits>

namespace im3e {

template <typename T>
struct PropertyValueConfig
{
    using Type = T;
    const std::string_view name;
    const std::string_view description;
    std::optional<T> defaultValue;
};

// clang-format off
template <typename T> struct IsPropertyValueConfigT : std::false_type{};
template <typename T> struct IsPropertyValueConfigT<PropertyValueConfig<T>> : std::true_type{};
template <typename T> inline constexpr bool IsPropertyValueConfig = IsPropertyValueConfigT<T>::value;

template <const auto& Config> 
concept CPropertyValueConfig = IsPropertyValueConfig<std::remove_cvref_t<decltype(Config)>>;
// clang-format on

template <const auto& Config>
    requires CPropertyValueConfig<Config>
class PropertyValue : public IPropertyValue
{
public:
    using Type = typename std::remove_cvref_t<decltype(Config)>::Type;

    void registerOnChange(std::weak_ptr<std::function<void()>> pOnChangeCallback) const override
    {
        m_changeNotifier.registerOnChange(std::move(pOnChangeCallback));
    }

    void setValue(Type value)
    {
        if (value != m_value)
        {
            m_value = std::move(value);
            m_changeNotifier.notifyChanged();
        }
    }

    void setAnyValue(std::any anyValue) override
    {
        auto value = std::any_cast<Type>(anyValue);
        setValue(std::move(value));
    }

    auto getName() const -> std::string_view override { return Config.name; }
    auto getType() const -> std::type_index override { return typeid(Type); }
    auto getValue() const -> Type { return m_value; }
    auto getAnyValue() const -> std::any { return m_value; }

private:
    mutable PropertyChangeNotifier m_changeNotifier;
    Type m_value = Config.defaultValue.value_or(Type{});
};

auto createPropertyGroup(std::string_view name, std::vector<std::shared_ptr<IProperty>> pProperties)
    -> std::shared_ptr<IPropertyGroup>;

}  // namespace im3e
