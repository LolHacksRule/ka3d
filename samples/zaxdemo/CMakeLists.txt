project(zaxdemo)

file(GLOB ZAXDEMO_SRC "*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/samples/zaxdemo)

add_executable(zaxdemo WIN32 ${ZAXDEMO_SRC})
target_include_directories(zaxdemo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../gameobjects ${CMAKE_CURRENT_SOURCE_DIR}/../../source/ode/ode-0.5/include ${CMAKE_CURRENT_SOURCE_DIR}/../../source/ode/ode-0.5/OPCODE)
add_dependencies(zaxdemo math-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})
#target_link_libraries(zaxdemo math-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})

target_link_libraries(zaxdemo math-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE} hgr-${MODULE_BUILD_TYPE} framework-${MODULE_BUILD_TYPE} gr-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} ode-${MODULE_BUILD_TYPE} gameobjects grdx-${MODULE_BUILD_TYPE}) #Target literally everything, unofficial but it builds