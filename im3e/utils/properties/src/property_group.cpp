#include "properties.h"

#include "property_change_notifier.h"

using namespace im3e;
using namespace std;

namespace {

class PropertyGroup : public IPropertyGroup
{
public:
    PropertyGroup(string_view name, vector<shared_ptr<IProperty>> pProperties)
      : m_name(name)
      , m_pProperties(move(pProperties))
    {
    }

    void registerOnChange(weak_ptr<function<void()>> pOnChangeCallback) const override
    {
        m_notifier.registerOnChange(move(pOnChangeCallback));
    }

    auto getName() const -> string override { return m_name; }
    auto getChildren() const -> vector<shared_ptr<IProperty>> override { return m_pProperties; }

private:
    const string m_name;
    vector<shared_ptr<IProperty>> m_pProperties;
    mutable PropertyChangeNotifier m_notifier;
};

}  // namespace

auto im3e::createPropertyGroup(string_view name, vector<shared_ptr<IProperty>> pProperties)
    -> shared_ptr<IPropertyGroup>
{
    return make_shared<PropertyGroup>(name, move(pProperties));
}