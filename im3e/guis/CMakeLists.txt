add_library(im3e_guis
    guis.h
    src/imgui_context.cpp
    src/imgui_context.h
    src/imgui_pipeline.cpp
    src/imgui_pipeline.h
    src/imgui_vulkan_backend.cpp
    src/imgui_vulkan_backend.h
    src/imgui_workspace.cpp
    src/imgui_workspace.h
)

target_include_directories(im3e_guis
  PUBLIC
    ${IM3E_INCLUDE_DIR}
  PRIVATE
    .
    src
)

target_link_libraries(im3e_guis
  PUBLIC
    im3e_api
    im3e_utils
  PRIVATE
    glfw
    im3e_resources
    imgui
)

add_subdirectory(test)
