#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace im3e {

/// @brief Internal utility class to help implement the notification system of properties.
class PropertyChangeNotifier
{
public:
    void registerOnChanged(std::weak_ptr<std::function<void()>> pOnChangedCallback);
    void notifyChanged();

private:
    std::vector<std::weak_ptr<std::function<void()>>> m_pOnChangeCallbacks;
};

}  // namespace im3e