file(GLOB GRDX_SRC "*.cpp")
add_library(grdx-${MODULE_BUILD_TYPE} STATIC ${GRDX_SRC})