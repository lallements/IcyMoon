cmake_minimum_required(VERSION 3.30.1)

project(PackageTest CXX)

find_package(OpenGL REQUIRED) 
find_package(pxr REQUIRED CONFIG)

add_definitions(
  # To suppress a warning caused by TBB in OpenUSD
  -DTBB_SUPPRESS_DEPRECATED_MESSAGES=1
)

add_compile_options(
    # OpenUSD causes warnings because of its use of <ext/hash_set>
    # See this issue for more details: https://github.com/PixarAnimationStudios/OpenUSD/issues/1057
    -Wno-deprecated 
)

add_executable(example
    src/example.cpp
)

target_link_libraries(example
  PRIVATE
    hdx
    OpenGL::GL
    usdImagingGL
)
