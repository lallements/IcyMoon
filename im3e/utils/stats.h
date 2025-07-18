#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <string_view>

namespace im3e {

struct Span
{
    std::filesystem::path path;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
};

class IStatsReceiver
{
public:
    virtual ~IStatsReceiver() = default;

    virtual void onSpanAdded(Span span) = 0;
};

class IStatsProvider
{
public:
    virtual ~IStatsProvider() = default;

    virtual void addReceiver(std::shared_ptr<IStatsReceiver> pReceiver) = 0;
    virtual void removeReceiver(std::shared_ptr<IStatsReceiver> pReceiver) = 0;

    class IScopedSpan
    {
    public:
        virtual ~IScopedSpan() = default;
    };
    virtual auto startScopedSpan(std::string_view name) -> std::unique_ptr<IScopedSpan> = 0;
};

auto createStatsProvider() -> std::shared_ptr<IStatsProvider>;

}  // namespace im3e