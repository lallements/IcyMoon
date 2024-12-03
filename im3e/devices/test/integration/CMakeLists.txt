im3e_add_integration_tests_executable(
  TARGET
    integration_im3e_devices
  SOURCES
    integration_vulkan_command_buffers.cpp
    integration_vulkan_device.cpp
    integration_vulkan_images.cpp
)

target_link_libraries(integration_im3e_devices
  PRIVATE
    im3e_devices
    im3e_test_utils
)
