im3e_add_unit_tests_executable(
  TARGET 
    test_im3e_devices
  SOURCES
    mock_vulkan_helper.h
    test_vulkan_command_buffers.cpp
    test_vulkan_debug_message_handler.cpp
    test_vulkan_extensions.cpp
    test_vulkan_images.cpp
    test_vulkan_instance.cpp
)

target_include_directories(test_im3e_devices
  PRIVATE
    ..
)

target_link_libraries(test_im3e_devices
  PRIVATE
    fmt::fmt
    im3e_devices
    im3e_test_utils
    mock_im3e
)

add_subdirectory(integration)