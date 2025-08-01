add_library(imgui STATIC
    imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_glfw.h
    imgui/backends/imgui_impl_vulkan.cpp
    imgui/backends/imgui_impl_vulkan.h
    imgui/misc/cpp/imgui_stdlib.cpp
    imgui/misc/cpp/imgui_stdlib.h

    # TODO: bring back implot
    #implot/implot_demo.cpp
    #implot/implot_internal.h
    #implot/implot_items.cpp
    #implot/implot.cpp
    #implot/implot.h
)

target_include_directories(imgui
  PUBLIC
    imgui
    imgui/backends
    imgui/misc/cpp
    implot
)

target_link_libraries(imgui
  PUBLIC
    glfw
    Vulkan::Headers
)

target_compile_definitions(imgui
  PUBLIC
    IMGUI_IMPL_VULKAN_NO_PROTOTYPES
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Suppress warning that causes compilation errors in imgui with GCC:
    target_compile_options(imgui
      PRIVATE
        -Wno-strict-aliasing
    )
endif()
