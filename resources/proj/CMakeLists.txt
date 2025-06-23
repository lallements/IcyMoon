option(PROJ_RES_PATH "Path to proj res folder where proj.db file for GDAL is located")
if(NOT PROJ_RES_PATH)
    message(FATAL_ERROR "PROJ_RES_PATH, required by GDAL, is not defined")
endif()

set(dstFolder "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_custom_target(im3e_resources_proj
  COMMAND
    ${CMAKE_COMMAND} -E copy "${PROJ_RES_PATH}/proj.db" ${dstFolder}
  DEPENDS
    "${PROJ_RES_PATH}/proj.db"
  BYPRODUCTS
    "${dstFolder}/proj.db"
  COMMENT
    "Copying proj.db from \"${PROJ_RES_PATH}\" to \"${dstFolder}\""
)
