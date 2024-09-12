project(draw_triangle)

file(GLOB DRAW_TRIANGLE_SRC "*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/samples/tutorials/draw_triangle)

add_executable(draw_triangle WIN32 ${DRAW_TRIANGLE_SRC})
add_dependencies(draw_triangle math-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} hgr-${MODULE_BUILD_TYPE} gr-${MODULE_BUILD_TYPE} grdx-${MODULE_BUILD_TYPE} framework-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})
target_link_libraries(draw_triangle math-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} hgr-${MODULE_BUILD_TYPE} gr-${MODULE_BUILD_TYPE} grdx-${MODULE_BUILD_TYPE} framework-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})