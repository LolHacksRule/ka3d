project(postprocessing_glow)

file(GLOB POSTPROCESSING_GLOW_SRC "*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/samples/tutorials/postprocessing_glow)

add_executable(postprocessing_glow WIN32 ${POSTPROCESSING_GLOW_SRC})
target_link_libraries(postprocessing_glow math-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} gr-${MODULE_BUILD_TYPE} grdx-${MODULE_BUILD_TYPE} hgr-${MODULE_BUILD_TYPE} ode-${MODULE_BUILD_TYPE} framework-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})