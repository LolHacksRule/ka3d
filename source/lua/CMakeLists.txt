project(lua)

file(GLOB LUA_SRC "*.cpp" "lua-5.0.2/src/*.cpp" "lua-5.0.2/src/lib/*.cpp")

add_compile_definitions(USE_FLOAT LUA_USER_H="${CMAKE_CURRENT_SOURCE_DIR}/lua-5.0.2/etc/luser_number.h")

add_library(lua-${MODULE_BUILD_TYPE} STATIC ${LUA_SRC})

target_include_directories(lua-${MODULE_BUILD_TYPE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lua-5.0.2/include)