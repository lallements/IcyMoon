cmake_minimum_required(VERSION 3.30.1)

project(PackageTest CXX)

find_package(open_usd REQUIRED)

add_executable(example
    src/example.cpp
)

target_link_libraries(example
  PRIVATE
    hdx
    usdImaging
    usdImagingGL
)
