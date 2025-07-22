add_library(im3e_geo STATIC
    geo.h
    src/gdal_geotiff_loader.cpp
    src/gdal_geotiff_loader.h
    src/gdal_instance.cpp
    src/gdal_instance.h
)

target_include_directories(im3e_geo
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(im3e_geo
  PUBLIC
    im3e_api
  PRIVATE
    GDAL::GDAL
    im3e_utils
)