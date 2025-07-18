add_library(im3e_utils STATIC
    src/platform_utils.cpp
    src/stats_provider.cpp
    src/stream_logger.cpp
    src/stream_logger.h
    src/vk_utils.cpp
    imgui_utils.h
    loggers.h
    throw_utils.h
    platform_utils.h
    stats.h
    types.h
    vk_utils.h
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
  PRIVATE
    whereami::whereami
)

add_subdirectory(mock)
add_subdirectory(test)
