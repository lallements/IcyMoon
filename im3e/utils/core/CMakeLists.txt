add_library(im3e_utils_core STATIC
    src/platform_utils.cpp
    platform_utils.h
    throw_utils.h
)

target_include_directories(im3e_utils_core
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
)

target_link_libraries(im3e_utils_core
  PUBLIC
    fmt::fmt
  PRIVATE
    whereami::whereami
)

add_subdirectory(test)
