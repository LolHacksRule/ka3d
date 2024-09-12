project(hgr)

file(GLOB HGR_SRC "*.cpp")

add_compile_definitions(hgr-${MODULE_BUILD_TYPE} _LIB)
add_library(hgr-${MODULE_BUILD_TYPE} STATIC ${HGR_SRC})