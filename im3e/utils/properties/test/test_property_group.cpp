#include "properties.h"

#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

namespace {

constexpr PropertyValueConfig<int32_t> IntPropertyConfig{.name = "Int Property"};
constexpr PropertyValueConfig<bool> BoolPropertyConfig{.name = "Bool Property"};

}  // namespace

TEST(PropertyGroupTest, constructor)
{
    constexpr auto TestGroupName = "Test Group";
    const vector<shared_ptr<IProperty>> properties{
        make_shared<PropertyValue<IntPropertyConfig>>(),
        make_shared<PropertyValue<BoolPropertyConfig>>(),
        createPropertyGroup("Other Group", {}),
    };
    auto pProperty = createPropertyGroup(TestGroupName, properties);

    EXPECT_THAT(pProperty->getName(), StrEq(TestGroupName));
    EXPECT_THAT(pProperty->getChildren(), ContainerEq(properties));
}