cmake_minimum_required(VERSION 3.16)
project(rexash-avsar)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

# Install custom module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_subdirectory(src)