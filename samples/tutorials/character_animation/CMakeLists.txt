project(character_animation)

file(GLOB CHARACTER_ANIMATION_SRC "*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/samples/tutorials/character_animation)

add_executable(character_animation WIN32 ${CHARACTER_ANIMATION_SRC})
target_link_libraries(character_animation math-${MODULE_BUILD_TYPE} lua-${MODULE_BUILD_TYPE} lang-${MODULE_BUILD_TYPE} io-${MODULE_BUILD_TYPE} img-${MODULE_BUILD_TYPE})