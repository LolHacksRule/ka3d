project(sceneviewer)
file(GLOB SCENEVIEWER_SRC "*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/build/msvc7/sceneviewer)

add_executable(sceneviewer WIN32 ${SCENEVIEWER_SRC})

target_include_directories(sceneviewer PRIVATE ../../include/sceneviewer)
target_include_directories(sceneviewer PRIVATE ../../build/msvc17/sceneviewer)
#add_compile_definitions(sceneviewer _AFXDLL _WIN32_WINNT_WINXP)
target_link_libraries(sceneviewer math-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})