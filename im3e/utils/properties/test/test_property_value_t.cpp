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

TEST(PropertyValueTTest, constructor)
{
    static constexpr PropertyValueTConfig<bool> BoolConfig{
        .name = "Boolean",
        .description = "Description of test property",
    };
    PropertyValueT<BoolConfig> property;
    EXPECT_THAT(property.getName(), StrEq(BoolConfig.name));
    EXPECT_THAT(property.getDescription(), StrEq(BoolConfig.description));
    EXPECT_THAT(property.getType() == typeid(bool), IsTrue());
    EXPECT_THAT(property.getValue(), Eq(bool{}));
}

TEST(PropertyValueTTest, constructorWithDefaultValue)
{
    static constexpr PropertyValueTConfig<uint32_t> UintConfig{
        .name = "Uint",
        .defaultValue = 42U,
    };
    PropertyValueT<UintConfig> property;
    EXPECT_THAT(property.getName(), StrEq(UintConfig.name));
    EXPECT_THAT(property.getType() == typeid(uint32_t), IsTrue());
    EXPECT_THAT(property.getValue(), Eq(UintConfig.defaultValue.value()));
}

TEST(PropertyValueTTest, setValue)
{
    static constexpr PropertyValueTConfig<float> FloatConfig{
        .defaultValue = 2.0F,
    };
    PropertyValueT<FloatConfig> property;
    EXPECT_THAT(property.getValue(), FloatEq(2.0F));

    OnChangeReceiver receiver(property);

    const float expectedValue = 5.5F;
    property.setValue(expectedValue);

    EXPECT_THAT(receiver.callCount, Eq(1U));
    EXPECT_THAT(property.getValue(), FloatEq(expectedValue));
}

TEST(PropertyValueTTest, setValueDoesNotNotifyIfGivenSameValue)
{
    constexpr uint32_t TestValue = 50U;

    static constexpr PropertyValueTConfig<uint32_t> IntConfig{
        .defaultValue = TestValue,
    };
    PropertyValueT<IntConfig> property;
    EXPECT_THAT(property.getValue(), Eq(TestValue));

    OnChangeReceiver receiver(property);

    property.setValue(TestValue);

    EXPECT_THAT(receiver.callCount, Eq(0U));
    EXPECT_THAT(property.getValue(), Eq(TestValue));
}

TEST(PropertyValueTTest, setAnyValue)
{
    static constexpr PropertyValueTConfig<int32_t> IntConfig{
        .defaultValue = -5,
    };
    PropertyValueT<IntConfig> property;
    EXPECT_THAT(any_cast<int32_t>(property.getAnyValue()), Eq(IntConfig.defaultValue.value()));

    OnChangeReceiver receiver(property);

    const int32_t expectedValue = 55;
    property.setAnyValue(expectedValue);

    EXPECT_THAT(receiver.callCount, Eq(1U));
    EXPECT_THAT(any_cast<int32_t>(property.getAnyValue()), Eq(expectedValue));
}

TEST(PropertyValueTTest, setAnyValueDoesNotNotifyIfGivenSameValue)
{
    constexpr int32_t TestValue = 65;

    static constexpr PropertyValueTConfig<int32_t> IntConfig{
        .defaultValue = TestValue,
    };
    PropertyValueT<IntConfig> property;
    EXPECT_THAT(any_cast<int32_t>(property.getAnyValue()), Eq(TestValue));

    OnChangeReceiver receiver(property);

    property.setAnyValue(TestValue);

    EXPECT_THAT(receiver.callCount, Eq(0U));
    EXPECT_THAT(any_cast<int32_t>(property.getAnyValue()), Eq(TestValue));
}
