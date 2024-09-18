#pragma once

#include <functional>
#include <memory>

namespace im3e {

template <typename T>
using UniquePtrWithDeleter = std::unique_ptr<T, std::function<void(T*)>>;

}  // namespace im3e