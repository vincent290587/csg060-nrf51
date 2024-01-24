cmake_minimum_required(VERSION 3.27)
project(SDK_LIBS LANGUAGES C ASM)

set(SDK_INC
        ${SDK_ROOT}/components/drivers_nrf/gpiote
        ${SDK_ROOT}/components/drivers_nrf/clock
        ${SDK_ROOT}/components/drivers_nrf/common
        ${SDK_ROOT}/components/drivers_nrf/rtc
        ${SDK_ROOT}/components/drivers_nrf/ppi
        ${SDK_ROOT}/components/drivers_nrf/wdt
        ${SDK_ROOT}/components/libraries/pwr_mgmt
        ${SDK_ROOT}/components/libraries/experimental_section_vars
        ${SDK_ROOT}/components/libraries/timer
        ${SDK_ROOT}/components
        ${SDK_ROOT}/components/libraries/util
        ${SDK_ROOT}/components/toolchain/gcc
        ${SDK_ROOT}/components/drivers_nrf/uart
        ${SDK_ROOT}/external/segger_rtt
        ${SDK_ROOT}/components/drivers_nrf/common
        ${SDK_ROOT}/components/libraries/fifo
        ${SDK_ROOT}/components/libraries/bsp
        ${SDK_ROOT}/components/libraries/scheduler
        ${SDK_ROOT}/components/toolchain
        ${SDK_ROOT}/components/libraries/uart
        ${SDK_ROOT}/components/device
        ${SDK_ROOT}/components/libraries/log
        ${SDK_ROOT}/components/boards
        ${SDK_ROOT}/components/drivers_nrf/delay
        ${SDK_ROOT}/components/toolchain/cmsis/include
        ${SDK_ROOT}/components/drivers_nrf/hal
        ${SDK_ROOT}/components/libraries/log/src
        ${SDK_ROOT}/components/libraries/fstorage
        ${CMAKE_SOURCE_DIR}/src/config
)

if (USE_SOFTDEVICE)

    list(APPEND SDK_INC
            ${SDK_ROOT}/components/softdevice/s130/headers
            ${SDK_ROOT}/components/softdevice/s130/headers/nrf51
            ${SDK_ROOT}/components/softdevice/common/softdevice_handler

            ${SDK_ROOT}/components/ble/nrf_ble_qwr
            ${SDK_ROOT}/components/ble/peer_manager
            ${SDK_ROOT}/components/ble/ble_services/ble_nus
            ${SDK_ROOT}/components/ble/ble_advertising
            ${SDK_ROOT}/components/ble/common
    )

else()

    list(APPEND SDK_INC
            ${SDK_ROOT}/components/drivers_nrf/nrf_soc_nosd
)

endif(USE_SOFTDEVICE)

include_directories(${SDK_INC})

set(SRC
    ${SDK_ROOT}/components/drivers_nrf/gpiote/nrf_drv_gpiote.c
    ${SDK_ROOT}/components/drivers_nrf/clock/nrf_drv_clock.c
    ${SDK_ROOT}/components/drivers_nrf/common/nrf_drv_common.c
    ${SDK_ROOT}/components/drivers_nrf/rtc/nrf_drv_rtc.c
    ${SDK_ROOT}/components/drivers_nrf/wdt/nrf_drv_wdt.c
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_backend_serial.c
    ${SDK_ROOT}/components/libraries/log/src/nrf_log_frontend.c
    ${SDK_ROOT}/components/libraries/util/app_error.c
    ${SDK_ROOT}/components/libraries/fifo/app_fifo.c
    ${SDK_ROOT}/components/libraries/scheduler/app_scheduler.c
    ${SDK_ROOT}/components/libraries/util/app_util_platform.c
    ${SDK_ROOT}/components/drivers_nrf/ppi/nrf_drv_ppi.c
    ${SDK_ROOT}/components/libraries/fstorage/fstorage.c
    ${SDK_ROOT}/components/libraries/timer/app_timer.c
    ${SDK_ROOT}/components/libraries/timer/app_timer_appsh.c
    ${SDK_ROOT}/components/libraries/util/nrf_assert.c
    ${SDK_ROOT}/components/libraries/uart/retarget.c
    ${SDK_ROOT}/components/libraries/util/sdk_errors.c
    ${SDK_ROOT}/components/libraries/timer/app_timer.c
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

if (USE_SOFTDEVICE)

    list(APPEND SRC
            ${SDK_ROOT}/components/softdevice/common/softdevice_handler/softdevice_handler.c
            ${SDK_ROOT}/components/softdevice/common/softdevice_handler/softdevice_handler_appsh.c

            ${SDK_ROOT}/components/ble/common/ble_advdata.c
            ${SDK_ROOT}/components/ble/ble_advertising/ble_advertising.c
            ${SDK_ROOT}/components/ble/common/ble_conn_params.c
            ${SDK_ROOT}/components/ble/common/ble_srv_common.c
            ${SDK_ROOT}/components/ble/ble_services/ble_nus/ble_nus.c
            ${SDK_ROOT}/components/ble/common/ble_advdata.c
            ${SDK_ROOT}/components/ble/common/ble_conn_params.c
            ${SDK_ROOT}/components/ble/common/ble_conn_state.c
            ${SDK_ROOT}/components/ble/common/ble_srv_common.c
    )

endif(USE_SOFTDEVICE)

add_library(SDK_LIBS ${SRC})

target_link_libraries(SDK_LIBS PUBLIC csg060_config)

target_include_directories(SDK_LIBS PUBLIC ${SDK_INC})
