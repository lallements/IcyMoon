add_library(im3e_loggers STATIC
    loggers.h
    src/logger_tracker.h
    src/loggers.cpp
    src/stream_logger.cpp
    src/stream_logger.h
)

target_include_directories(im3e_loggers
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(im3e_loggers
  PUBLIC
    im3e_api
  PRIVATE
    fmt::fmt
    im3e_utils
)

add_subdirectory(test)
