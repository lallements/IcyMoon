set(srcFile "SourceCodePro-Regular.ttf")
set(dstFolder "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fonts")

file(MAKE_DIRECTORY ${dstFolder})

add_custom_target(im3e_resources_fonts
  COMMAND
    ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${srcFile}" ${dstFolder}
  DEPENDS
    SourceCodePro-Regular.ttf
  BYPRODUCTS
    "${dstFolder}/${srcFile}"
  COMMENT
    "Copying \"${srcFile}\" to \"${dstFolder}\""
)

add_dependencies(im3e_resources im3e_resources_fonts)