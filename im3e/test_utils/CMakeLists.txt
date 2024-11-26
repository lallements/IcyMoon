add_library(im3e_test_utils STATIC
    device_integration_test.h
    integration_test.h
    pipeline_integration_test.h
    std.h
    test_utils.h
    vk.h
    src/device_integration_test.cpp
    src/integration_test.cpp
    src/pipeline_integration_test.cpp
    src/vk.cpp
)

target_include_directories(im3e_test_utils
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(im3e_test_utils
  PUBLIC
    fmt::fmt
    GTest::gmock
    GTest::gtest_main
    im3e_api
    im3e_devices
    im3e_guis
    im3e_loggers
    im3e_utils
)

im3e_exclude_from_coverage()