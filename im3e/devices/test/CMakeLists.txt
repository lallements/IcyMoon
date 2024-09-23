im3e_add_unit_tests_executable(
  TARGET 
    test_im3e_devices
  SOURCES
    test_vulkan_instance.cpp
)

target_include_directories(test_im3e_devices
  PRIVATE
    ..
    ../src
)

target_link_libraries(test_im3e_devices
  PRIVATE
    fmt::fmt
    GTest::gmock
    GTest::gtest_main
    im3e_devices
    mock_im3e
)

add_subdirectory(integration)