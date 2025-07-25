#include "stats.h"

#include <im3e/utils/core/throw_utils.h>

#include <mutex>
#include <optional>
#include <set>
#include <stack>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace im3e;
using namespace std;
using namespace std::chrono;

namespace {

class StatsProvider : public IStatsProvider, public enable_shared_from_this<StatsProvider>
{
public:
    void addReceiver(shared_ptr<IStatsReceiver> pReceiver) override
    {
        throwIfArgNull(pReceiver, "Cannot add null receiver to stats provider");

        lock_guard lk(m_mutex);
        m_pReceivers.emplace(move(pReceiver));
    }

    void removeReceiver(shared_ptr<IStatsReceiver> pReceiver) override
    {
        throwIfArgNull(pReceiver, "Cannot remove null receiver from stats provider");

        lock_guard lk(m_mutex);
        m_pReceivers.erase(pReceiver);
    }

    class ScopedSpan : public IScopedSpan
    {
    public:
        ScopedSpan(shared_ptr<StatsProvider> pProvider, filesystem::path spanPath)
          : m_pProvider(throwIfArgNull(move(pProvider), "ScopedSpan requires a StatsProvider"))
          , m_span(Span{
                .path = move(spanPath),
                .startTime = steady_clock::now(),
            })
        {
        }

        ~ScopedSpan() override
        {
            m_span.endTime = steady_clock::now();
            {
                lock_guard lk(m_pProvider->m_mutex);
                for (auto& pReceiver : m_pProvider->m_pReceivers)
                {
                    pReceiver->onSpanAdded(m_span);
                }

                auto& rActiveSpans = m_pProvider->m_threadToActiveSpans[this_thread::get_id()];
                throwIfFalse<logic_error>(!rActiveSpans.empty() && m_span.path == rActiveSpans.top(),
                                          "Incorrect active span being removed");
                rActiveSpans.pop();
            }
        }

    private:
        shared_ptr<StatsProvider> m_pProvider;
        Span m_span;
    };
    auto startScopedSpan(string_view name) -> unique_ptr<IScopedSpan> override
    {
        lock_guard lk(m_mutex);
        auto& rActiveSpans = m_threadToActiveSpans[this_thread::get_id()];
        auto spanPath = (rActiveSpans.empty() ? m_rootPath : rActiveSpans.top()) / name;
        rActiveSpans.emplace(spanPath);
        return make_unique<ScopedSpan>(this->shared_from_this(), move(spanPath));
    }

private:
    const filesystem::path m_rootPath{"/", filesystem::path::generic_format};
    mutex m_mutex;
    set<shared_ptr<IStatsReceiver>> m_pReceivers;
    unordered_map<thread::id, stack<filesystem::path>> m_threadToActiveSpans;
};

}  // namespace

auto im3e::createStatsProvider() -> shared_ptr<IStatsProvider>
{
    return make_unique<StatsProvider>();
}