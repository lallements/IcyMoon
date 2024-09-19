add_library(im3e_api INTERFACE
    device.h
    vulkan_loader.h
)

target_include_directories(im3e_api
  INTERFACE
    ${IM3E_INCLUDE_DIR}
)

target_link_libraries(im3e_api
  INTERFACE
    vulkan-headers::vulkan-headers  
)