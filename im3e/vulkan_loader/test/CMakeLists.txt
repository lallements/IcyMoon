im3e_add_unit_tests_executable(
  TARGET
    test_im3e_vulkan_loader
  SOURCES
    test_vulkan_loader.cpp
)

target_link_libraries(test_im3e_vulkan_loader
  PRIVATE
    GTest::gmock
    GTest::gtest_main
    im3e_vulkan_loader
)
