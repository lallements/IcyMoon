#pragma once

#include "property_change_notifier.h"

#include <im3e/utils/properties/api/property.h>

namespace im3e {

template <typename Desc>
class Property : public IProperty
{
public:
    void registerOnChange(std::weak_ptr<std::function<void()>> pOnChangeCallback) const override
    {
        m_changeNotifier.registerOnChanged(std::move(pOnChangeCallback));
    }

    auto getName() const -> std::string { return Desc::Name; }
    auto getType() const -> std::type_index { return typeid(typename Desc::Type); }

private:
    mutable PropertyChangeNotifier m_changeNotifier;
};

}  // namespace im3e
