project(using_transforms)

file(GLOB USING_TRANSFORMS_SRC "*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/samples/tutorials/using_transforms)

add_executable(using_transforms WIN32 ${USING_TRANSFORMS_SRC})
target_link_libraries(using_transforms math-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} gr-${MODULE_BUILD_TYPE} grdx-${MODULE_BUILD_TYPE} hgr-${MODULE_BUILD_TYPE} ode-${MODULE_BUILD_TYPE} framework-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})