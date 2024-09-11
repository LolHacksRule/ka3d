file(GLOB IO_SRC "*.cpp" "std/*.cpp" "zlib-1.2.1/*.c")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.1)

add_library(io-${MODULE_BUILD_TYPE} STATIC ${IO_SRC})