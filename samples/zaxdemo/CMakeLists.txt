project(zaxdemo)

file(GLOB ZAXDEMO_SRC "*.cpp")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gameobjects)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/samples/zaxdemo)

add_executable(zaxdemo ${ZAXDEMO_SRC})
target_link_libraries(zaxdemo math-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})