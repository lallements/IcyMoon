add_library(im3e_resources INTERFACE)

target_include_directories(im3e_resources
  INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(im3e_resources
  INTERFACE
    im3e_utils_core
)

set(IM3E_RESOURCES_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/resources.h.in" 
  "${CMAKE_CURRENT_BINARY_DIR}/im3e/resources/resources.h" 
  @ONLY
)

add_subdirectory(fonts)