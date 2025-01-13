add_library(im3e_usd
    usd.h
    src/hydra_frame_pipeline.cpp
)

target_include_directories(im3e_usd
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(im3e_usd
  PUBLIC
    im3e_api
  PRIVATE
    im3e_utils
)

add_subdirectory(test)