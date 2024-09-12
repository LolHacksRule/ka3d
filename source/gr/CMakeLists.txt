project(gr)

file(GLOB GR_SRC "*.cpp")

add_subdirectory(dx)

add_compile_definitions(gr-${MODULE_BUILD_TYPE} _LIB)
add_library(gr-${MODULE_BUILD_TYPE} STATIC ${GR_SRC})