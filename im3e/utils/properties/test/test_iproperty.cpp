#include <im3e/utils/properties/api/property.h>

#include <im3e/test_utils/test_utils.h>

#include <gmock/gmock.h>

#include <string>

using namespace im3e;
using namespace std;

namespace {

class TestProperty : public IProperty
{
public:
    void registerOnChange(weak_ptr<function<void()>>) const override {}

    void setAnyValue(any) override {}

    auto getName() const -> string_view override { return ""; }
    auto getType() const -> type_index override { return typeid(void); }
    auto getAnyValue() const -> any override { return {}; }
};

}  // namespace

TEST(IPropertyTest, getChildren)
{
    TestProperty property;
    EXPECT_THAT(property.getChildren().empty(), IsTrue());
}
