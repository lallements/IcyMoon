#pragma once

namespace {

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

}  // namespace