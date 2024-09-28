im3e_add_unit_tests_executable(
    TARGET test_im3e_loggers
    SOURCES 
      test_logger_tracker.cpp
      test_loggers.cpp
      test_stream_logger_global_tracker.cpp
      test_stream_logger.cpp
)

target_include_directories(test_im3e_loggers
  PRIVATE
    .
    ..
)

target_link_libraries(test_im3e_loggers
  PRIVATE
    fmt::fmt
    im3e_loggers
    im3e_test_utils
    mock_im3e
)
