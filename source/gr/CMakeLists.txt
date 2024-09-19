project(gr)

file(GLOB GR_SRC "*.cpp")

if(WIN32)
	add_subdirectory(dx)
endif()

add_compile_definitions(_LIB)
add_library(gr-${MODULE_BUILD_TYPE} STATIC ${GR_SRC})