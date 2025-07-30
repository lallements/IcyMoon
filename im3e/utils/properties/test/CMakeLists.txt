im3e_add_unit_tests_executable(
  TARGET
    test_im3e_utils_properties
  SOURCES
    test_property_change_notifier.cpp
    test_property_group.cpp
    test_property_value.cpp
)

target_include_directories(test_im3e_utils_properties
  PRIVATE
    ..
)

target_link_libraries(test_im3e_utils_properties
  PRIVATE
    im3e_test_utils
    im3e_utils_properties
)
