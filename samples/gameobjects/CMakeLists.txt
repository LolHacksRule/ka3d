file(GLOB GAMEOBJECTS_SRC "*.cpp")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ode/ode-0.5/include ${CMAKE_CURRENT_SOURCE_DIR}/../../source/ode/ode-0.5/OPCODE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/samples/gameobjects)

add_library(gameobjects STATIC ${GAMEOBJECTS_SRC})