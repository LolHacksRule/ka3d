project(lightmappacker)

file(GLOB LIGHTMAPPACKER_SRC "*.cpp")
add_library(lightmappacker STATIC ${LIGHTMAPPACKER_SRC})