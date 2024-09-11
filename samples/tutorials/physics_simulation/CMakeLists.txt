project(physics_simulation)

file(GLOB PHYSICS_SIMULATION_SRC "*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/samples/tutorials/physics_simulation)

add_executable(physics_simulation WIN32 ${PHYSICS_SIMULATION_SRC})
target_link_libraries(physics_simulation math-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} gr-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE} hgr-${MODULE_BUILD_TYPE} ode-${MODULE_BUILD_TYPE} grdx-${MODULE_BUILD_TYPE} framework-${MODULE_BUILD_TYPE})