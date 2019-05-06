include_directories("../../3rdparty/mongoose")
include_directories("../../3rdparty/mbedtls/include")

aux_source_directory(. MA_SOURCE)

add_definitions(-DMG_ENABLE_SSL -DMG_ENABLE_CALLBACK_USERDATA -DMG_ENABLE_SYNC_RESOLVER)

if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

list(APPEND LINK_LIB 
mongoose 
mbedcrypto 
mbedtls 
mbedx509 
)

if(WIN32)
    list(APPEND LINK_LIB ws2_32)
else()
    list(APPEND LINK_LIB pthread dl m )
endif()

if(ANDROID)
    list(REMOVE_ITEM LINK_LIB pthread)
endif()

if(ANDROID)
    #error: Android 5.0 and later only support position-independent executables (-fPIE).
    list(APPEND LINK_LIB "-fPIE -pie")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
endif()

add_executable(mirroraccel ${MA_SOURCE})
target_link_libraries(mirroraccel ${LINK_LIB})

add_library(libmirroraccel SHARED ${MA_SOURCE} ${MA_RESOURCE})
target_link_libraries(libmirroraccel ${LINK_LIB})
target_compile_definitions (libmirroraccel PRIVATE "MA_EXPORTS")