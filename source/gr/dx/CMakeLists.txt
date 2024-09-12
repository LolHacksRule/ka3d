project(grdx)

file(GLOB GRDX_SRC "*.cpp")
add_compile_definitions(_LIB)
add_library(grdx-${MODULE_BUILD_TYPE} STATIC ${GRDX_SRC})