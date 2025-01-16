im3e_add_unit_tests_executable(
  TARGET
    test_im3e_usd
  SOURCES
    test_im3e_usd.cpp
)

target_include_directories(test_im3e_usd
  PRIVATE
    .
)

target_link_libraries(test_im3e_usd
  PRIVATE
    hdx
    im3e_resources
    im3e_test_utils
    usdImaging
    usdImagingGL
)