add_library(im3e_utils_properties_api INTERFACE
    property.h
)

target_include_directories(im3e_utils_properties_api
  INTERFACE
    ${IM3E_INCLUDE_DIR}
)

target_link_libraries(im3e_utils_properties_api
  INTERFACE
    fmt::fmt
)
