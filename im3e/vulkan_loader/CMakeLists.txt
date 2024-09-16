add_library(im3e_vulkan_loader STATIC
    vulkan_loader.h
    src/vulkan_loader.cpp
)

target_include_directories(im3e_vulkan_loader
  PUBLIC
    ..
  PRIVATE
    .
    src
)

target_link_libraries(im3e_vulkan_loader
  PUBLIC
    im3e_api
)

add_subdirectory(test)