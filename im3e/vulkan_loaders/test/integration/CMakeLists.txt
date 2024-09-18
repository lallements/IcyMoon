im3e_add_integration_tests_executable(
  TARGET
    integration_im3e_vulkan_loaders
  SOURCES
    integration_vulkan_loaders.cpp
)

target_link_libraries(integration_im3e_vulkan_loaders
  PRIVATE
    GTest::gmock
    GTest::gtest_main
    im3e_vulkan_loaders
)