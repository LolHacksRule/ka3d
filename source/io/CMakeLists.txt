project(io)

file(GLOB IO_SRC "*.cpp" "std/*.cpp" "zlib-1.2.1/*.c")

if(WIN32)
    file(GLOB IO_WIN32_SRC "win32/*.cpp")
    list(APPEND IO_SRC ${IO_WIN32_SRC})
endif()

add_compile_definitions(_LIB)
add_library(io-${MODULE_BUILD_TYPE} STATIC ${IO_SRC})
target_include_directories(io-${MODULE_BUILD_TYPE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.2.1)