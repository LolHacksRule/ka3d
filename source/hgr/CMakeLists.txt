file(GLOB HGR_SRC "*.cpp")
add_library(hgr-${MODULE_BUILD_TYPE} STATIC ${HGR_SRC})