#include "properties.h"

#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

namespace {

struct MyProperty : public Property<MyProperty>
{
    using Type = uint32_t;
    static constexpr auto Name = "MyProperty";
    static constexpr auto Description = "Unsigned integer value for testing properties";
};

}  // namespace

TEST(PropertyTest, constructor)
{
    MyProperty myProp;
    EXPECT_THAT(myProp.getName(), StrEq(MyProperty::Name));
    EXPECT_THAT(myProp.getType() == typeid(MyProperty::Type), IsTrue());
    EXPECT_THAT(myProp.getChildren(), IsEmpty());
}