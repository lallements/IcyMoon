#include "properties.h"

#include <im3e/test_utils/test_utils.h>

using namespace im3e;
using namespace std;

namespace {

struct OnChangeReceiver
{
    OnChangeReceiver(IProperty& rProperty) { rProperty.registerOnChange(pOnChange); }

    uint32_t callCount{};
    shared_ptr<function<void()>> pOnChange = make_shared<function<void()>>([this] { callCount++; });
};

}  // namespace

TEST(PropertyValueTest, constructor)
{
    static constexpr PropertyValueConfig<bool> BoolConfig{
        .name = "Boolean",
    };
    PropertyValue<BoolConfig> property;
    EXPECT_THAT(property.getName(), StrEq(BoolConfig.name));
    EXPECT_THAT(property.getType() == typeid(bool), IsTrue());
    EXPECT_THAT(property.getValue(), Eq(bool{}));
}

TEST(PropertyValueTest, constructorWithDefaultValue)
{
    static constexpr PropertyValueConfig<uint32_t> UintConfig{
        .name = "Uint",
        .defaultValue = 42U,
    };
    PropertyValue<UintConfig> property;
    EXPECT_THAT(property.getName(), StrEq(UintConfig.name));
    EXPECT_THAT(property.getType() == typeid(uint32_t), IsTrue());
    EXPECT_THAT(property.getValue(), Eq(UintConfig.defaultValue.value()));
}

TEST(PropertyValueTest, setValue)
{
    static constexpr PropertyValueConfig<float> FloatConfig{
        .defaultValue = 2.0F,
    };
    PropertyValue<FloatConfig> property;
    EXPECT_THAT(property.getValue(), FloatEq(2.0F));

    OnChangeReceiver receiver(property);

    const float expectedValue = 5.5F;
    property.setValue(expectedValue);

    EXPECT_THAT(receiver.callCount, Eq(1U));
    EXPECT_THAT(property.getValue(), FloatEq(expectedValue));
}

TEST(PropertyValueTest, setValueDoesNotNotifyIfGivenSameValue)
{
    constexpr uint32_t TestValue = 50U;

    static constexpr PropertyValueConfig<uint32_t> IntConfig{
        .defaultValue = TestValue,
    };
    PropertyValue<IntConfig> property;
    EXPECT_THAT(property.getValue(), Eq(TestValue));

    OnChangeReceiver receiver(property);

    property.setValue(TestValue);

    EXPECT_THAT(receiver.callCount, Eq(0U));
    EXPECT_THAT(property.getValue(), Eq(TestValue));
}

TEST(PropertyValueTest, setAnyValue)
{
    static constexpr PropertyValueConfig<int32_t> IntConfig{
        .defaultValue = -5,
    };
    PropertyValue<IntConfig> property;
    EXPECT_THAT(any_cast<int32_t>(property.getAnyValue()), Eq(IntConfig.defaultValue.value()));

    OnChangeReceiver receiver(property);

    const int32_t expectedValue = 55;
    property.setAnyValue(expectedValue);

    EXPECT_THAT(receiver.callCount, Eq(1U));
    EXPECT_THAT(any_cast<int32_t>(property.getAnyValue()), Eq(expectedValue));
}

TEST(PropertyValueTest, setAnyValueDoesNotNotifyIfGivenSameValue)
{
    constexpr int32_t TestValue = 65;

    static constexpr PropertyValueConfig<int32_t> IntConfig{
        .defaultValue = TestValue,
    };
    PropertyValue<IntConfig> property;
    EXPECT_THAT(any_cast<int32_t>(property.getAnyValue()), Eq(TestValue));

    OnChangeReceiver receiver(property);

    property.setAnyValue(TestValue);

    EXPECT_THAT(receiver.callCount, Eq(0U));
    EXPECT_THAT(any_cast<int32_t>(property.getAnyValue()), Eq(TestValue));
}
