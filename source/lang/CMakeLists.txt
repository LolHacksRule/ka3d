file(GLOB LANG_SRC "*.cpp" "*.c")
add_library(lang-${MODULE_BUILD_TYPE} STATIC ${LANG_SRC})