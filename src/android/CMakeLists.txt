cmake_minimum_required(VERSION 3.18.1)

project("particlesystem_performancetest")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -frtti -fexceptions -Wall")
set(OPENGL_LIB GLESv3)

add_library(
        particlesystem_performancetest
        SHARED
        jni_bridge.cpp
        device/android_file_handler.cpp
        ../common/gl/gl_util.cpp
        ../common/test_app.cpp
        ../common/particle_system/cpu_serial_particle_system.cpp
        ../common/particle_system/cpu_serial_instance_particle_system.cpp
        ../common/particle_system/cpu_parallel_particle_system.cpp
        ../common/particle_system/tf_particle_system.cpp
        ../common/particle_system/cs_particle_system.cpp
        ../common/util/random.cpp

        ../common/particle_system/cpu_module_color_over_lifetime.cpp
        ../common/particle_system/cpu_module_velocity_over_lifetime.cpp
        ../common/particle_system/cpu_module_emission.cpp
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
