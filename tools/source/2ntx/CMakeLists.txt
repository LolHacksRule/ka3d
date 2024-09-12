project(2ntx)

#Uses HGR and GR
file(GLOB 2NTX_SRC "*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/build/msvc17/2ntx)

add_executable(2ntx WIN32 ${2NTX_SRC})
add_dependencies(2ntx gr-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})
target_link_libraries(2ntx gr-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})