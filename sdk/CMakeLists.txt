cmake_minimum_required(VERSION 3.27)
project(SDK_LIBS LANGUAGES C ASM)


set(SRC
    ${SDK_ROOT}/components/drivers_nrf/gpiote/nrf_drv_gpiote.c
    ${SDK_ROOT}/components/drivers_nrf/clock/nrf_drv_clock.c
    ${SDK_ROOT}/components/drivers_nrf/common/nrf_drv_common.c
    ${SDK_ROOT}/components/drivers_nrf/rtc/nrf_drv_rtc.c
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_backend_serial.c
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_frontend.c
    ${SDK_ROOT}/components/libraries/util/app_error.c
    ${SDK_ROOT}/components/libraries/fifo/app_fifo.c
    ${SDK_ROOT}/components/libraries/util/app_util_platform.c
    ${SDK_ROOT}/components/libraries/util/nrf_assert.c
    ${SDK_ROOT}/components/libraries/uart/retarget.c
    ${SDK_ROOT}/components/libraries/util/sdk_errors.c
    ${SDK_ROOT}/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c
    ${SDK_ROOT}/components/libraries/hardfault/nrf51/handler/hardfault_handler_gcc.c
    ${SDK_ROOT}/components/libraries/hardfault/hardfault_implementation.c
    ${SDK_ROOT}/components/boards/boards.c
    ${SDK_ROOT}/components/drivers_nrf/common/nrf_drv_common.c
    ${SDK_ROOT}/components/drivers_nrf/uart/nrf_drv_uart.c
    ${SDK_ROOT}/external/segger_rtt/RTT_Syscalls_GCC.c
    ${SDK_ROOT}/external/segger_rtt/SEGGER_RTT.c
    ${SDK_ROOT}/external/segger_rtt/SEGGER_RTT_printf.c
    ${SDK_ROOT}/components/toolchain/gcc/gcc_startup_nrf51.S
    ${SDK_ROOT}/components/toolchain/system_nrf51.c
    ${CMAKE_CURRENT_SOURCE_DIR}/qemu_rtt.c
)

add_library(SDK_LIBS ${SRC})

target_include_directories(SDK_LIBS PUBLIC
    ${SDK_ROOT}/components/drivers_nrf/gpiote
    ${SDK_ROOT}/components/drivers_nrf/clock
    ${SDK_ROOT}/components/drivers_nrf/common
    ${SDK_ROOT}/components/drivers_nrf/rtc
    ${SDK_ROOT}/components/libraries/pwr_mgmt
    ${SDK_ROOT}/components/libraries/experimental_section_vars
    ${SDK_ROOT}/components
    ${SDK_ROOT}/components/libraries/util
    ${SDK_ROOT}/components/toolchain/gcc
    ${SDK_ROOT}/components/drivers_nrf/uart
    ${SDK_ROOT}/external/segger_rtt
    ${SDK_ROOT}/components/drivers_nrf/common
    ${SDK_ROOT}/components/libraries/fifo
    ${SDK_ROOT}/components/libraries/bsp
    ${SDK_ROOT}/components/drivers_nrf/nrf_soc_nosd
    ${SDK_ROOT}/components/toolchain
    ${SDK_ROOT}/components/libraries/uart
    ${SDK_ROOT}/components/device
    ${SDK_ROOT}/components/libraries/log
    ${SDK_ROOT}/components/boards
    ${SDK_ROOT}/components/drivers_nrf/delay
    ${SDK_ROOT}/components/toolchain/cmsis/include
    ${SDK_ROOT}/components/drivers_nrf/hal
    ${SDK_ROOT}/components/libraries/log/src
    ${CMAKE_SOURCE_DIR}/src/config
)

target_link_libraries(SDK_LIBS PUBLIC csg060_config)
