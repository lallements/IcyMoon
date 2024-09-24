add_library(im3e_utils INTERFACE
    throw_utils.h
    types.h
    vk_utils.h
)

target_include_directories(im3e_utils
  INTERFACE
    ${IM3E_INCLUDE_DIR}
)

target_link_libraries(im3e_utils
  INTERFACE
    fmt::fmt
    Vulkan::Headers 
)
