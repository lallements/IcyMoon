add_library(im3e_anari STATIC
    anari.h
    src/anari_device.cpp
    src/anari_device.h
    src/anari_extension_utility.cpp
)

target_include_directories(im3e_anari
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(im3e_anari
  PUBLIC
    anari::anari_static
    im3e_utils
  PRIVATE
    fmt::fmt
)