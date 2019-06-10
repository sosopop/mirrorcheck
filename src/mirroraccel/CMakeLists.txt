include_directories("../../3rdparty/mongoose")
include_directories("../../3rdparty/nlohmann")
include_directories("../../3rdparty/mbedtls/include")
include_directories("../../3rdparty/curl/include")
include_directories("../../3rdparty/spdlog/include")

aux_source_directory(. MA_SOURCE)

add_definitions(-DCURL_STATICLIB -DMG_ENABLE_SSL -DMG_ENABLE_CALLBACK_USERDATA -DMG_ENABLE_SYNC_RESOLVER)

if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8")
endif()

if(UNIX)
if(NOT ANDROID)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()
endif()

list(APPEND LINK_LIB 
curl 
zlibstatic
mbedcrypto 
mbedtls 
mbedx509 
cares 
mongoose 
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
    #list(APPEND LINK_LIB "-fPIE -pie")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")
endif()

if(WIN32)
add_executable(mirroraccel ${MA_SOURCE})
target_link_libraries(mirroraccel ${LINK_LIB})
else()
add_library(mirroraccel SHARED ${MA_SOURCE} ${MA_RESOURCE})
target_link_libraries(mirroraccel ${LINK_LIB})
target_compile_definitions (mirroraccel PRIVATE "MA_EXPORTS")
endif()