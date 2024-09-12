project(grdx)

file(GLOB GRDX_SRC "*.cpp")
add_compile_definitions(grdx-${MODULE_BUILD_TYPE} _LIB)
add_library(grdx-${MODULE_BUILD_TYPE} STATIC ${GRDX_SRC})