file(GLOB GR_SRC "*.cpp")

add_subdirectory(dx)

add_library(gr-${MODULE_BUILD_TYPE} STATIC ${GR_SRC})