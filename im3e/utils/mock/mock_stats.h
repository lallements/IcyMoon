#pragma once

#include <im3e/test_utils/test_utils.h>
#include <im3e/utils/stats.h>

namespace im3e {

class MockStatsReceiver : public IStatsReceiver
{
public:
    MockStatsReceiver();
    ~MockStatsReceiver() override;

    MOCK_METHOD(void, onSpanAdded, (Span span), (override));
};

class MockStatsProvider : public IStatsProvider
{
public:
    MockStatsProvider();
    ~MockStatsProvider() override;

    MOCK_METHOD(void, addReceiver, (std::shared_ptr<IStatsReceiver> pReceiver), (override));
    MOCK_METHOD(void, removeReceiver, (std::shared_ptr<IStatsReceiver> pReceiver), (override));

    MOCK_METHOD(std::unique_ptr<IScopedSpan>, startScopedSpan, (std::string_view name), (override));

    auto createMockProxy() -> std::unique_ptr<IStatsProvider>;
};

}  // namespace im3e