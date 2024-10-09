add_library(im3e_utils STATIC
    throw_utils.h
    types.h
    vk_utils.h
    src/vk_utils.cpp
)

target_include_directories(im3e_utils
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(im3e_utils
  PUBLIC
    fmt::fmt
    vulkan-memory-allocator::vulkan-memory-allocator
    Vulkan::Headers
)

add_subdirectory(test)
