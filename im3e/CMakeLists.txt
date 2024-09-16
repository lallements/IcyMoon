add_library(im3e_api INTERFACE
    api/vulkan_loader.h
)

target_include_directories(im3e_api
  INTERFACE
    ..
)

add_subdirectory(vulkan_loader)
