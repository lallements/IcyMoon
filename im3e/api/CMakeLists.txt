add_library(im3e_api INTERFACE
    command_buffer.h
    device.h
    frame_pipeline.h
    gui.h
    image.h
    logger.h
    vulkan_loader.h
)

target_include_directories(im3e_api
  INTERFACE
    ${IM3E_INCLUDE_DIR}
)

target_link_libraries(im3e_api
  INTERFACE
    fmt::fmt
    im3e_utils  
)