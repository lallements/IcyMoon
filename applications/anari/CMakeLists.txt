find_package(anari)
if (NOT anari_FOUND)
    message(WARNING "Could not find Anari SDK, skipping anari app")
    return()
endif()

message(STATUS "Found anari, building anari app")

add_executable(anari
    anari_frame_pipeline.cpp
    anari_frame_pipeline.h
    anari.cpp
)

target_link_libraries(anari
  PRIVATE
    anari::anari_static
    fmt::fmt
    im3e_devices
    im3e_guis
    im3e_utils
)

