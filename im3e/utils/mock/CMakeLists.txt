im3e_add_mock_library(
  TARGET
    mock_im3e_utils
  SOURCES
    src/mock_logger.cpp
    mock_logger.h
)

target_include_directories(mock_im3e_utils
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(mock_im3e_utils
  PUBLIC
    GTest::gmock
    im3e_utils
)
