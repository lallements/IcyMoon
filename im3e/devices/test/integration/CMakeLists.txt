im3e_add_integration_tests_executable(
  TARGET
    integration_im3e_devices
  SOURCES
    integration_device.cpp
)

target_link_libraries(integration_im3e_devices
  PRIVATE
    GTest::gmock
    GTest::gtest_main
    im3e_devices
)
