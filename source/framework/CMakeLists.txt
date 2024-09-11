file(GLOB FRAMEWORK_SRC "*.cpp")

if(WIN32)
    #file(GLOB FRAMEWORK_WIN32_SRC "win32/*.cpp")
    list(APPEND FRAMEWORK_SRC "win32/WinMain_dx.cpp") #WinMain is excluded
#elseif(${PLATFORM} STREQUAL "Symbian")
#    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/symbian)
#    file(GLOB FRAMEWORK_SYMBIAN_SRC "*.cpp")
#    list(APPEND FRAMEWORK_SRC ${FRAMEWORK_SYMBIAN_SRC})
#elseif(${PLATFORM} STREQUAL "NGI")
#    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/NGI)
#    file(GLOB FRAMEWORK_NGI_SRC "*.cpp")
#    list(APPEND FRAMEWORK_SRC ${FRAMEWORK_NGI_SRC})
endif()

add_library(framework-${MODULE_BUILD_TYPE} STATIC ${FRAMEWORK_SRC})