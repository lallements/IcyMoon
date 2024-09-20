add_library(im3e_devices STATIC
    devices.h
    src/device.cpp
    src/device.h
    src/vulkan_instance.cpp
    src/vulkan_instance.h
)

target_include_directories(im3e_devices
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(im3e_devices
  PUBLIC
    im3e_api
  PRIVATE
    im3e_utils
    im3e_vulkan_loaders
)

add_subdirectory(test)