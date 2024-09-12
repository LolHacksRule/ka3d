project(tools)

include_directories(${CMAKE_SOURCE_DIR}/tools/include)

add_subdirectory(source/lightmappacker)
#add_subdirectory(source/maxexport)
add_subdirectory(source/sceneviewer)
add_subdirectory(source/2ntx)