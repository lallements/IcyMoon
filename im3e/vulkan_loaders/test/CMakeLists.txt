im3e_add_unit_tests_executable(
  TARGET
    test_im3e_vulkan_loaders
  SOURCES
    test_vulkan_loaders.cpp
)

target_include_directories(test_im3e_vulkan_loaders
  PRIVATE
    ..
)

target_link_libraries(test_im3e_vulkan_loaders
  PRIVATE
    fmt::fmt
    GTest::gmock
    GTest::gtest_main
    im3e_vulkan_loaders
)

add_subdirectory(integration)
