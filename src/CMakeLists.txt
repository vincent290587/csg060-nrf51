cmake_minimum_required(VERSION 3.27)
project(csg060_fw)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE        OFF)
set(CMAKE_COLOR_MAKEFILE          OFF)


add_compile_options(-fsingle-precision-constant)
add_compile_options(-Wdouble-promotion -Wfloat-conversion -Werror=return-type)
add_compile_options(-Wno-attributes -Wsign-compare -Wenum-compare -Wno-error=deprecated)
#add_compile_options(-Wno-register)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/log
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/drivers AUX_SRCS)

add_executable(csg060_fw
        main.c
        os_time.c
        ble_app.c
        dfu_mgmt.c
        ${AUX_SRCS}
)

# !! Important ! need to use target_...
target_link_options(${PROJECT_NAME} PRIVATE -L${SDK_ROOT}/components/toolchain/gcc)
target_link_options(${PROJECT_NAME} PRIVATE -T${CMAKE_SOURCE_DIR}/pca10028/blank/armgcc/uart_gcc_nrf51.ld)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/csg060_fw.map)

target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--whole-archive SDK_LIBS -Wl,--no-whole-archive)
target_link_libraries(${PROJECT_NAME} PRIVATE --specs=nano.specs)
target_link_libraries(${PROJECT_NAME} PRIVATE nosys)

if (USE_QEMU)
target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--defsym=SEGGER_RTT_WriteNoLock=SEGGER_RTT_WriteNoLock_qemu)
target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--defsym=SEGGER_RTT_WriteSkipNoLock=SEGGER_RTT_WriteSkipNoLock_qemu)
endif(USE_QEMU)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        BYPRODUCTS ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex ${PROJECT_NAME}.bin
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.elf
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
        COMMENT "Building ${HEX_FILE} Building ${BIN_FILE}"
        COMMAND ${CMAKE_SIZE_UTIL} --format=berkeley ${PROJECT_NAME}
        COMMENT "Invoking: Print Size"
)

find_program(NRFUTIL "nrfutil")
find_program(NRFJPROG "nrfjprog")
find_program(MERGEHEX "mergehex" HINTS "c:/Program Files (x86)/Nordic Semiconductor/nrf5x/bin")

if(NOT NRFJPROG)
    message(FATAL_ERROR "nrfjprog not found, not able to merge connectivity firmware with SoftDevice.")
endif()

if(NOT MERGEHEX)
    message(FATAL_ERROR "mergehex not found, not able to merge connectivity firmware with SoftDevice.")
endif()

if(NOT NRFUTIL)
    message(FATAL_ERROR "nrfutil not found, not able to create DFU packages of connectivity firmware.")
endif()

if (NOT USE_SOFTDEVICE)
    add_custom_target(flash
            DEPENDS ${PROJECT_NAME}.hex
            # Build app first
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
            #Then program
            COMMAND ${NRFJPROG} --program ${PROJECT_NAME}.hex -f nrf51 --sectorerase
            COMMAND ${NRFJPROG} --reset -f nrf51
    )
endif(NOT USE_SOFTDEVICE)
