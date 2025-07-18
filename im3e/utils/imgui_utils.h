#pragma once

#include "throw_utils.h"

#include <functional>

namespace im3e {

class ImguiScope
{
public:
    ImguiScope(bool isOpen, std::function<void()> endScopeFct, bool alwaysEndScope = false)
      : m_isOpen(isOpen)
      , m_endScopeFct(throwIfArgNull(std::move(endScopeFct), "ImGui scope requires an end of scope function"))
      , m_alwaysEndScope(alwaysEndScope)
    {
    }

    ImguiScope(std::function<void()> endScopeFct)
      : ImguiScope(true, std::move(endScopeFct), true)
    {
    }

    ~ImguiScope()
    {
        if (m_isOpen || m_alwaysEndScope)
        {
            m_endScopeFct();
        }
    }

    bool isOpen() const { return m_isOpen; }

    operator bool() const { return m_isOpen; }

private:
    const bool m_isOpen = false;
    std::function<void()> m_endScopeFct;
    const bool m_alwaysEndScope = false;
};

}  // namespace im3e