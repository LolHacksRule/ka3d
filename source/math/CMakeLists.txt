file(GLOB MATH_SRC "*.cpp" "std/*.cpp")

add_library(math-${MODULE_BUILD_TYPE} STATIC ${MATH_SRC})