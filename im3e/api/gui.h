#pragma once

namespace im3e {

class IGuiPanel
{
public:
    virtual ~IGuiPanel() = default;
};

class IGuiWorkspace : public IGuiPanel
{
public:
    virtual ~IGuiWorkspace() = default;
};

}  // namespace im3e