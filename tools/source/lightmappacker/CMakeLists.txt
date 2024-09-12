project(lightmappacker)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/build/msvc17/lightmappacker)

file(GLOB LIGHTMAPPACKER_SRC "*.cpp")
add_compile_definitions(_LIB)
add_library(lightmappacker STATIC ${LIGHTMAPPACKER_SRC})