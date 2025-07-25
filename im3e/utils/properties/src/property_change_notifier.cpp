#include "property_change_notifier.h"

#include <im3e/utils/core/throw_utils.h>

using namespace im3e;
using namespace std;

void PropertyChangeNotifier::registerOnChanged(weak_ptr<function<void()>> pOnChangedCallback)
{
    throwIfArgNull(pOnChangedCallback.lock(), "Property cannot register null callback");
    throwIfArgNull(*pOnChangedCallback.lock(), "Property cannot register null callback");
    m_pOnChangeCallbacks.emplace_back(move(pOnChangedCallback));
}

void PropertyChangeNotifier::notifyChanged()
{
    auto it = m_pOnChangeCallbacks.begin();
    while (it != m_pOnChangeCallbacks.end())
    {
        if (auto pFct = it->lock(); pFct && *pFct)
        {
            (*pFct)();
            it++;
        }
        else
        {
            it = m_pOnChangeCallbacks.erase(it);
        }
    }
}
