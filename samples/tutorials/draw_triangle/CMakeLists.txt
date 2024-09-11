project(draw_triangle)

file(GLOB DRAW_TRIANGLE_SRC "*.cpp")

add_executable(draw_triangle WIN32 ${DRAW_TRIANGLE_SRC})
target_link_libraries(draw_triangle math-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} hgr-${MODULE_BUILD_TYPE} gr-${MODULE_BUILD_TYPE} grdx-${MODULE_BUILD_TYPE} framework-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})