#pragma once

#include <gmock/gmock.h>

namespace im3e {

using ::testing::_;
using ::testing::ContainerEq;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::IsEmpty;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsSubsetOf;
using ::testing::IsSupersetOf;
using ::testing::IsTrue;
using ::testing::MockFunction;
using ::testing::Ne;
using ::testing::NiceMock;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Test;
using ::testing::Unused;
using ::testing::Values;
using ::testing::ValuesIn;

}  // namespace im3e