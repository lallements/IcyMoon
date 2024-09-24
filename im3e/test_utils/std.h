#pragma once

#include <fmt/format.h>
#include <gmock/gmock.h>

#include <chrono>

namespace std {

template <typename T>
void PrintTo(std::chrono::duration<T> duration, std::ostream* pStream)
{
    *pStream << fmt::format("{}s", duration.count());
}

template <typename T>
void PrintTo(std::chrono::duration<T, std::milli> duration, std::ostream* pStream)
{
    *pStream << fmt::format("{}ms", duration.count());
}

template <typename T>
void PrintTo(std::chrono::duration<T, std::micro> duration, std::ostream* pStream)
{
    *pStream << fmt::format("{}us", duration.count());
}

template <typename T>
void PrintTo(std::chrono::duration<T, std::nano> duration, std::ostream* pStream)
{
    *pStream << fmt::format("{}ns", duration.count());
}

}  // namespace std