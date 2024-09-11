cmake_minimum_required(VERSION 3.10)
project(ka3d)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MODULE_BUILD_TYPE "mdd") #Debug
else()
    set(MODULE_BUILD_TYPE "md") #Release
endif()

link_directories($ENV{DXSDK_DIR}/Lib/x86) #DirectX 9/+ SDK libs must be installed, won't be distributed, otherwise modify this
include_directories($ENV{DXSDK_DIR}/Include) #DirectX 9/+ SDK must be installed, won't be distributed, otherwise modify this

#link_directories(${CMAKE_SOURCE_DIR}/lib/msvc17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/msvc17)

add_compile_definitions(_CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_WARNINGS)

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tools/include ${CMAKE_SOURCE_DIR}/include/config/msvc17)

add_subdirectory(source)
add_subdirectory(samples) #Not yet buildable
add_subdirectory(tools) #Not yet buildable