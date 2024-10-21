add_library(im3e_test_utils STATIC
    std.h
    test_utils.h
    vk.h
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
    im3e_utils
)

im3e_exclude_from_coverage()