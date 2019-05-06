cmake_minimum_required(VERSION 3.0.0)

include_directories("../mbedtls/include")

add_definitions(-DMG_ENABLE_SSL -DMG_SSL_IF=MG_SSL_IF_MBEDTLS -DMG_SSL_MBED_DUMMY_RANDOM -DMG_ENABLE_THREADS -DMG_ENABLE_CALLBACK_USERDATA -DMG_ENABLE_SYNC_RESOLVER -DFD_SETSIZE=256)
add_library(mongoose mongoose.c)