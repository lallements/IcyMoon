#pragma once

#include "command_buffer.h"

#include <memory>
#include <string>

namespace im3e {

class IGuiPanel
{
public:
    virtual ~IGuiPanel() = default;

    virtual void draw(const ICommandBuffer& rCommandBuffer) = 0;

    /// @brief Called whenever the window this panel belongs to is resized.
    /// @param[in] rWindowSize New window size
    /// @param[in] vkFormat Format of the window
    /// @param[in] frameInFlightCount Number of frames that can be in flight for rendering at a time.
    virtual void onWindowResized([[maybe_unused]] const VkExtent2D& rWindowSize, [[maybe_unused]] VkFormat vkFormat,
                                 [[maybe_unused]] uint32_t frameInFlightCount)
    {
    }

    virtual auto getName() const -> std::string = 0;
};

class IGuiWorkspace : public IGuiPanel
{
public:
    virtual ~IGuiWorkspace() = default;

    enum class Location
    {
        Top,
        Bottom,
        Center,
        Left,
        Right
    };
    virtual void addPanel(Location location, std::shared_ptr<IGuiPanel> pPanel, float fraction = 0.25F) = 0;
};

}  // namespace im3e