project(scene_animation)

file(GLOB SCENE_ANIMATION_SRC "*.cpp")

add_executable(scene_animation WIN32 ${SCENE_ANIMATION_SRC})
target_link_libraries(scene_animation math-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} gr-${MODULE_BUILD_TYPE} grdx-${MODULE_BUILD_TYPE} hgr-${MODULE_BUILD_TYPE} ode-${MODULE_BUILD_TYPE} framework-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})