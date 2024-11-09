#pragma once

#include "command_buffer.h"

namespace im3e {

class IGuiPanel
{
public:
    virtual ~IGuiPanel() = default;

    virtual void draw(const ICommandBuffer& rCommandBuffer) = 0;

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