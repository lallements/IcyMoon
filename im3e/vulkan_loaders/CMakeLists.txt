add_library(im3e_vulkan_loaders STATIC
    vulkan_loaders.h
    src/vulkan_loader.cpp
    src/vulkan_loader.h
    "$<$<PLATFORM_ID:Linux>:src/vulkan_library_loader_linux.h>"
)

target_include_directories(im3e_vulkan_loaders
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(im3e_vulkan_loaders
  PUBLIC
    im3e_api
)

add_subdirectory(test)