cmake_minimum_required(VERSION 3.27)
project(csg060 VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

message(STATUS "Project version: ${CMAKE_PROJECT_VERSION}")

set(USE_SOFTDEVICE                OFF)
set(USE_QEMU                      ON)

if (USE_QEMU)
    add_definitions(-DNRF_LOG_BACKEND_RTT_ENABLED=1)
    add_definitions(-DNRF_LOG_BACKEND_UART_ENABLED=0)
    add_definitions(-DNRF_LOG_USES_COLORS=0)
    add_definitions(-DNRF_LOG_DEFAULT_LEVEL=3)
    add_definitions(-DNRF_USE_QEMU)

    set(USE_SOFTDEVICE                OFF)
    set(USE_CONNECTIVITY              OFF)

endif(USE_QEMU)


add_subdirectory(config/)

add_subdirectory(sdk/)

add_subdirectory(src/)

file( GENERATE OUTPUT "${CMAKE_BINARY_DIR}/.gitignore" CONTENT "*" )
