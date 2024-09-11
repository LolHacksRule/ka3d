file(GLOB IMG_SRC "*.cpp") #Why is 2ntx from tools here

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/jpeg-6b ${CMAKE_CURRENT_SOURCE_DIR}/Devil-SDK-1.6.7/include ${CMAKE_CURRENT_SOURCE_DIR}/libpng-1.2.5 ${CMAKE_CURRENT_SOURCE_DIR}/../io/zlib-1.2.1)

set(IMG_EXTERNAL_SRC
    jpeg-6b/jcapimin.cpp
    jpeg-6b/jcapistd.cpp
    jpeg-6b/jccoefct.cpp
    jpeg-6b/jccolor.cpp
    jpeg-6b/jcdctmgr.cpp
    jpeg-6b/jchuff.cpp
    jpeg-6b/jcinit.cpp
    jpeg-6b/jcmainct.cpp
    jpeg-6b/jcmarker.cpp
    jpeg-6b/jcmaster.cpp
    jpeg-6b/jcomapi.cpp
    jpeg-6b/jcparam.cpp
    jpeg-6b/jcphuff.cpp
    jpeg-6b/jcprepct.cpp
    jpeg-6b/jcsample.cpp
    jpeg-6b/jctrans.cpp
    jpeg-6b/jdapimin.cpp
    jpeg-6b/jdapistd.cpp
    jpeg-6b/jdatadst.cpp
    jpeg-6b/jdatasrc.cpp
    jpeg-6b/jdcoefct.cpp
    jpeg-6b/jdcolor.cpp
    jpeg-6b/jddctmgr.cpp
    jpeg-6b/jdhuff.cpp
    jpeg-6b/jdinput.cpp
    jpeg-6b/jdmainct.cpp
    jpeg-6b/jdmarker.cpp
    jpeg-6b/jdmaster.cpp
    jpeg-6b/jdmerge.cpp
    jpeg-6b/jdphuff.cpp
    jpeg-6b/jdpostct.cpp
    jpeg-6b/jdsample.cpp
    jpeg-6b/jdtrans.cpp
    jpeg-6b/jerror.cpp
    jpeg-6b/jfdctflt.cpp
    jpeg-6b/jfdctfst.cpp
    jpeg-6b/jfdctint.cpp
    jpeg-6b/jidctflt.cpp
    jpeg-6b/jidctfst.cpp
    jpeg-6b/jidctint.cpp
    jpeg-6b/jidctred.cpp
    jpeg-6b/jmemmgr.cpp
    jpeg-6b/jmemnobs.cpp
    jpeg-6b/jquant1.cpp
    jpeg-6b/jquant2.cpp
    jpeg-6b/jutils.cpp
    libpng-1.2.5/png.c
    libpng-1.2.5/pngerror.c
    libpng-1.2.5/pngget.c
    libpng-1.2.5/pngmem.c
    libpng-1.2.5/pngpread.c
    libpng-1.2.5/pngread.c
    libpng-1.2.5/pngrio.c
    libpng-1.2.5/pngrtran.c
    libpng-1.2.5/pngrutil.c
    libpng-1.2.5/pngset.c
    libpng-1.2.5/pngtrans.c
    libpng-1.2.5/pngvcrd.c
    libpng-1.2.5/pngwio.c
    libpng-1.2.5/pngwrite.c
    libpng-1.2.5/pngwtran.c
    libpng-1.2.5/pngwutil.c
)

add_compile_definitions(IMG_JPG_SUPPORT IMG_PNG_SUPPORT IMG_IL_SUPPORT IMG_WRITER_SUPPORT)
add_library(img-${MODULE_BUILD_TYPE} STATIC ${IMG_SRC} ${IMG_EXTERNAL_SRC})
target_link_libraries(img-${MODULE_BUILD_TYPE} ${CMAKE_CURRENT_SOURCE_DIR}/Devil-SDK-1.6.7/lib/DevIL.lib)