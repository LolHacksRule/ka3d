project(particle_rendering)

file(GLOB PARTICLE_RENDERING_SRC "*.cpp")

add_executable(particle_rendering WIN32 ${PARTICLE_RENDERING_SRC})
target_link_libraries(particle_rendering math-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} gr-${MODULE_BUILD_TYPE} grdx-${MODULE_BUILD_TYPE} hgr-${MODULE_BUILD_TYPE} ode-${MODULE_BUILD_TYPE} framework-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})