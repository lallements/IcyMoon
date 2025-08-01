add_library(im3e_guis
    guis.h
    src/glfw_window.cpp
    src/glfw_window.h
    src/glfw_window_application.cpp
    src/glfw_window_application.h
    src/imgui_context.cpp
    src/imgui_context.h
    src/imgui_pipeline.cpp
    src/imgui_pipeline.h
    src/imgui_property_control.cpp
    src/imgui_property_control.h
    src/imgui_property_panel.cpp
    src/imgui_property_panel.h
    src/imgui_render_panel.cpp
    src/imgui_render_panel.h
    src/imgui_stats_panel.cpp
    src/imgui_stats_panel.h
    src/imgui_vulkan_backend.cpp
    src/imgui_vulkan_backend.h
    src/imgui_workspace.cpp
    src/imgui_workspace.h
    src/presenter.cpp
    src/presenter.h
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
    glm::glm
  PRIVATE
    glfw
    imgui
    im3e_devices
    im3e_resources
    im3e_utils_properties_api
)

add_subdirectory(test)
