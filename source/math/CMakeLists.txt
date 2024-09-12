file(GLOB MATH_SRC "*.cpp" "std/*.cpp")
add_compile_definitions(math-${MODULE_BUILD_TYPE} _LIB)
add_library(math-${MODULE_BUILD_TYPE} STATIC ${MATH_SRC})