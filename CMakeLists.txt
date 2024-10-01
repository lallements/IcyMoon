cmake_minimum_required(VERSION 3.30.1)

project (IcyMoonEngine VERSION 0.1.0)

enable_testing()
include(GoogleTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

set(IM3E_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Setting up ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
endif()

add_definitions(
    -DVK_NO_PROTOTYPES # for vulkan headers: we link dynamically and do not want vk function definitions

    # Vulkan Memory Allocator
    -DVMA_STATIC_VULKAN_FUNCTIONS=0
    -DVMA_DYNAMIC_VULKAN_FUNCTIONS=0
)

if (MSVC)
    # warning level 4 and all warnings as errors
    # system headers treated as external (i.e. no compiler warning on them)
    add_compile_options(/W4 /WX /experimental:external /external:W0 /MP)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -Wno-missing-field-initializers
        )
    endif()
endif()

add_subdirectory(cmake)
im3e_set_coverage_flags()

find_package(fmt REQUIRED)
find_package(GTest REQUIRED)
find_package(VulkanHeaders REQUIRED)


add_subdirectory(applications)
add_subdirectory(im3e)

im3e_init_code_coverage()
