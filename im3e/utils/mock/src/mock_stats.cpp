#include "mock_stats.h"

using namespace im3e;
using namespace std;

MockStatsReceiver::MockStatsReceiver() = default;
MockStatsReceiver::~MockStatsReceiver() = default;

namespace {

class MockProxyStatsProvider : public IStatsProvider
{
public:
    MockProxyStatsProvider(MockStatsProvider& rMock)
      : m_rMock(rMock)
    {
    }

    void addReceiver(shared_ptr<IStatsReceiver> pReceiver) override { m_rMock.addReceiver(move(pReceiver)); }
    void removeReceiver(shared_ptr<IStatsReceiver> pReceiver) override { m_rMock.removeReceiver(move(pReceiver)); }

    auto startScopedSpan(string_view name) -> unique_ptr<IScopedSpan> override { return m_rMock.startScopedSpan(name); }

private:
    MockStatsProvider& m_rMock;
};

}  // namespace

MockStatsProvider::MockStatsProvider() = default;
MockStatsProvider::~MockStatsProvider() = default;

auto MockStatsProvider::createMockProxy() -> unique_ptr<IStatsProvider>
{
    return make_unique<MockProxyStatsProvider>(*this);
}
