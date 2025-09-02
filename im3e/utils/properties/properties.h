#pragma once

#include "property_change_notifier.h"

#include <im3e/utils/properties/api/property.h>

#include <optional>
#include <type_traits>

namespace im3e {

template <typename T>
struct PropertyValueTConfig
{
    using Type = T;
    const std::string_view name;
    const std::string_view description;
    std::optional<T> defaultValue;
};

// clang-format off
template <typename T> struct IsPropertyValueTConfigT : std::false_type{};
template <typename T> struct IsPropertyValueTConfigT<PropertyValueTConfig<T>> : std::true_type{};
template <typename T> inline constexpr bool IsPropertyValueTConfig = IsPropertyValueTConfigT<T>::value;

template <const auto& Config> 
concept CPropertyValueTConfig = IsPropertyValueTConfig<std::remove_cvref_t<decltype(Config)>>;
// clang-format on

template <const auto& Config>
    requires CPropertyValueTConfig<Config>
class PropertyValueT : public IPropertyValue
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

    auto getName() const -> std::string override { return std::string{Config.name}; }
    auto getDescription() const -> std::string override { return std::string{Config.description}; }
    auto getType() const -> std::type_index override { return typeid(Type); }
    auto getValue() const -> Type { return m_value; }
    auto getAnyValue() const -> std::any { return m_value; }

private:
    mutable PropertyChangeNotifier m_changeNotifier;
    Type m_value = Config.defaultValue.value_or(Type{});
};

template <typename T>
struct PropertyValueConfig
{
    using Type = T;
    const std::string name;
    const std::string description;
    T defaultValue{};
    std::function<void(T newValue)> onChange{};
};

template <typename T>
class PropertyValue : public IPropertyValue
{
public:
    using Type = T;

    PropertyValue(PropertyValueConfig<T> config)
      : m_config(std::move(config))
      , m_value(config.defaultValue)
    {
    }

    void registerOnChange(std::weak_ptr<std::function<void()>> pOnChangeCallback) const override
    {
        m_notifier.registerOnChange(std::move(pOnChangeCallback));
    }

    void setAnyValue(std::any value) override
    {
        m_value = move(value);
        m_notifier.notifyChanged();
        if (m_config.onChange)
        {
            m_config.onChange(std::any_cast<T>(m_value));
        }
    }

    auto getName() const -> std::string override { return m_config.name; }
    auto getDescription() const -> std::string override { return m_config.description; }
    auto getType() const -> std::type_index { return typeid(Type); }
    auto getAnyValue() const -> std::any { return m_value; }
    auto getValue() const -> Type { return std::any_cast<Type>(m_value); }

private:
    PropertyValueConfig<T> m_config;
    mutable PropertyChangeNotifier m_notifier;
    std::any m_value;
};

auto createPropertyGroup(std::string_view name, std::vector<std::shared_ptr<IProperty>> pProperties)
    -> std::shared_ptr<IPropertyGroup>;

}  // namespace im3e
