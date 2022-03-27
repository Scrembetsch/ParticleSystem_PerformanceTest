cmake_minimum_required(VERSION 3.18.1)

project("particlesystem_performancetest")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-rtti -fno-exceptions -Wall")
set(OPENGL_LIB GLESv3)

add_library(
        particlesystem_performancetest
        SHARED
        jni_bridge.cpp
)

find_library(
        log-lib
        log )

target_link_libraries(
        particlesystem_performancetest
        ${log-lib}
        ${OPENGL_LIB}
        android
        EGL
        m
)
