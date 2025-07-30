#include "imgui_stats_panel.h"

#include "guis.h"

#include <im3e/utils/core/throw_utils.h>
#include <im3e/utils/imgui_utils.h>

#include <fmt/format.h>
#include <imgui.h>

#include <limits>

using namespace im3e;
using namespace std;
using namespace std::chrono;

namespace {

}

ImguiStatsPanel::ImguiStatsPanel(string_view name, shared_ptr<IStatsProvider> pStatsProvider)
  : m_name(name)
  , m_pStatsProvider(throwIfArgNull(std::move(pStatsProvider), "ImGui stats panel requires a stats provider"))
  , m_pStatsReceiver(make_shared<StatsReceiver>(*this))
{
    m_pStatsProvider->addReceiver(m_pStatsReceiver);
}

ImguiStatsPanel::~ImguiStatsPanel()
{
    m_pStatsProvider->removeReceiver(m_pStatsReceiver);
}

void ImguiStatsPanel::draw(const ICommandBuffer&)
{
    constexpr ImGuiTableFlags TableFlags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
                                           ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg;
    if (auto tableScope = ImguiScope(ImGui::BeginTable("Spans", 6, TableFlags), &ImGui::EndTable))
    {
        ImGui::TableSetupColumn("Span");
        ImGui::TableSetupColumn("Current");
        ImGui::TableSetupColumn("Min");
        ImGui::TableSetupColumn("Average");
        ImGui::TableSetupColumn("Max");
        ImGui::TableSetupColumn("Frequency");
        ImGui::TableHeadersRow();

        lock_guard lk(m_mutex);

        for (auto& [rSpanPath, rSpanStats] : m_spanStats)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", rSpanPath.c_str());

            microseconds curDuration{};
            microseconds minDuration{numeric_limits<microseconds::rep>::max()};
            microseconds totalDuration{};
            microseconds maxDuration{};
            float frequency{};

            constexpr auto TimeWindow = 1s;
            auto& rDurations = rSpanStats.durations;

            auto lowerBound = steady_clock::now() - TimeWindow;
            auto itLowerBound = rDurations.lower_bound(lowerBound);
            if (itLowerBound != rDurations.begin())
            {
                itLowerBound--;
                rDurations.erase(rDurations.begin(), itLowerBound);
            }

            if (!rDurations.empty())
            {
                curDuration = rDurations.rbegin()->second;

                for (auto& [rTimestamp, rDuration] : rDurations)
                {
                    minDuration = min(minDuration, rDuration);
                    totalDuration += rDuration;
                    maxDuration = max(maxDuration, rDuration);
                }

                const auto curTimeWindow = duration_cast<milliseconds>(rDurations.rbegin()->first -
                                                                       rDurations.begin()->first);
                frequency = rDurations.size() / (curTimeWindow.count() / 1.0e3F);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", fmt::format("{:.2f}ms", curDuration.count() / 1000.0).c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", fmt::format("{:.2f}ms", minDuration.count() / 1000.0).c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", fmt::format("{:.2f}ms", totalDuration.count() / (rDurations.size() * 1000.0)).c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", fmt::format("{:.2f}ms", maxDuration.count() / 1000.0).c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", fmt::format("{:.2f}", frequency).c_str());
        }
    }
}

ImguiStatsPanel::StatsReceiver::StatsReceiver(ImguiStatsPanel& rPanel)
  : m_rPanel(rPanel)
{
}

void ImguiStatsPanel::StatsReceiver::onSpanAdded(Span span)
{
    lock_guard lk(m_rPanel.m_mutex);
    auto& rSpanStats = m_rPanel.m_spanStats[span.path];
    rSpanStats.durations.emplace(steady_clock::now(), duration_cast<microseconds>(span.endTime - span.startTime));
}

auto im3e::createImguiStatsPanel(string_view name, shared_ptr<IStatsProvider> pStatsProvider) -> shared_ptr<IGuiPanel>
{
    return make_shared<ImguiStatsPanel>(name, move(pStatsProvider));
}
