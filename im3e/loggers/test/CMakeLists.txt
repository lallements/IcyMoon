im3e_add_unit_tests_executable(
    TARGET test_im3e_loggers
    SOURCES 
      test_loggers.cpp
      test_stream_logger_global_tracker.cpp
)

target_include_directories(test_im3e_loggers
  PRIVATE
    .
    ..
    ../src
)

target_link_libraries(test_im3e_loggers
  PRIVATE
    fmt::fmt
    GTest::gtest_main
    im3e_loggers
    mock_im3e
)
