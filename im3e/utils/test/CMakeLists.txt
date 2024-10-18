im3e_add_unit_tests_executable(
  TARGET
    test_im3e_utils
  SOURCES
    test_throw_utils.cpp
    test_types.cpp
    test_vk_utils.cpp
)

target_include_directories(test_im3e_utils
  PRIVATE
    ..
)

target_link_libraries(test_im3e_utils
  PRIVATE
    im3e_test_utils
    im3e_utils
    mock_im3e
)