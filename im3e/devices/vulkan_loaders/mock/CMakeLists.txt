im3e_add_mock_library(
  TARGET
    mock_im3e_vulkan_loaders
  SOURCES
    mock_vulkan_loader.cpp
    mock_vulkan_loader.h
)

target_include_directories(mock_im3e_vulkan_loaders
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(mock_im3e_vulkan_loaders
  PUBLIC
    GTest::gmock
    im3e_api
    mock_im3e
  PRIVATE
    im3e_utils
)
