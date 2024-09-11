project(sceneviewer)
include_directories(../../include/sceneviewer)
include_directories(../../build/msvc17/sceneviewer)
file(GLOB SCENEVIEWER_SRC "*.cpp")
add_executable(sceneviewer WIN32 ${SCENEVIEWER_SRC})

target_link_libraries(sceneviewer math-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})