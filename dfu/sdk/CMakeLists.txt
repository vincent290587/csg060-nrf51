cmake_minimum_required(VERSION 3.27)
project(SDK_DFU LANGUAGES C ASM)


set(SRC
    ${SDK_ROOT}/components/libraries/util/app_error_weak.c
    ${SDK_ROOT}/components/libraries/scheduler/app_scheduler.c
    ${SDK_ROOT}/components/libraries/timer/app_timer.c
    ${SDK_ROOT}/components/libraries/timer/app_timer_appsh.c
    ${SDK_ROOT}/components/libraries/util/app_util_platform.c
    ${SDK_ROOT}/components/libraries/crc32/crc32.c
    ${SDK_ROOT}/components/libraries/ecc/ecc.c
    ${SDK_ROOT}/components/libraries/fstorage/fstorage.c
    ${SDK_ROOT}/components/libraries/hci/hci_mem_pool.c
    ${SDK_ROOT}/components/libraries/util/nrf_assert.c
    ${SDK_ROOT}/components/libraries/crypto/nrf_crypto.c
    ${SDK_ROOT}/components/libraries/queue/nrf_queue.c
    ${SDK_ROOT}/components/libraries/util/sdk_errors.c
    ${SDK_ROOT}/components/libraries/sha256/sha256.c
    ${SDK_ROOT}/components/boards/boards.c
    ${SDK_ROOT}/components/drivers_nrf/common/nrf_drv_common.c
    ${SDK_ROOT}/components/drivers_nrf/rng/nrf_drv_rng.c
    ${SDK_ROOT}/components/drivers_nrf/hal/nrf_nvmc.c
    ${SDK_ROOT}/components/drivers_nrf/ppi/nrf_drv_ppi.c
    ${SDK_ROOT}/components/libraries/bootloader/ble_dfu/nrf_ble_dfu.c
    ${SDK_ROOT}/components/ble/common/ble_advdata.c
    ${SDK_ROOT}/components/ble/common/ble_conn_params.c
    ${SDK_ROOT}/components/ble/common/ble_srv_common.c
    ${SDK_ROOT}/external/nano-pb/pb_common.c
    ${SDK_ROOT}/external/nano-pb/pb_decode.c
    ${SDK_ROOT}/components/toolchain/gcc/gcc_startup_nrf51.S
    ${SDK_ROOT}/components/toolchain/system_nrf51.c
    ${SDK_ROOT}/components/softdevice/common/softdevice_handler/softdevice_handler.c
    ${SDK_ROOT}/components/softdevice/common/softdevice_handler/softdevice_handler_appsh.c
    ${SDK_ROOT}/components/libraries/bootloader/nrf_bootloader.c
    ${SDK_ROOT}/components/libraries/bootloader/nrf_bootloader_app_start.c
    ${SDK_ROOT}/components/libraries/bootloader/nrf_bootloader_info.c
    ${SDK_ROOT}/components/libraries/bootloader/dfu/nrf_dfu.c
    ${SDK_ROOT}/components/libraries/bootloader/dfu/nrf_dfu_flash.c
    ${SDK_ROOT}/components/libraries/bootloader/dfu/nrf_dfu_mbr.c
    ${SDK_ROOT}/components/libraries/bootloader/dfu/nrf_dfu_settings.c
    ${SDK_ROOT}/components/libraries/bootloader/dfu/nrf_dfu_transport.c
    ${SDK_ROOT}/components/libraries/bootloader/dfu/nrf_dfu_utils.c
)

add_library(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDK_ROOT}/components/drivers_nrf/rng
    ${SDK_ROOT}/components/device
    ${SDK_ROOT}/components/drivers_nrf/hal
    ${SDK_ROOT}/components/libraries/sha256
    ${SDK_ROOT}/components/libraries/crc32
    ${SDK_ROOT}/components/libraries/experimental_section_vars
    ${SDK_ROOT}/components/libraries/fstorage
    ${SDK_ROOT}/components/libraries/util
    ${SDK_ROOT}/components
    ${SDK_ROOT}/components/softdevice/common/softdevice_handler
    ${SDK_ROOT}/components/libraries/timer
    ${SDK_ROOT}/components/drivers_nrf/clock
    ${SDK_ROOT}/components/softdevice/s130/headers
    ${SDK_ROOT}/components/libraries/log/src
    ${SDK_ROOT}/components/libraries/bootloader/dfu
    ${SDK_ROOT}/components/drivers_nrf/delay
    ${SDK_ROOT}/components/drivers_nrf/ppi
    ${SDK_ROOT}/components/ble/common
    ${SDK_ROOT}/components/drivers_nrf/common
    ${SDK_ROOT}/components/libraries/svc
    ${SDK_ROOT}/components/libraries/scheduler
    ${SDK_ROOT}/components/libraries/log
    ${SDK_ROOT}/components/libraries/hci
    ${SDK_ROOT}/components/boards
    ${SDK_ROOT}/components/libraries/bootloader
    ${SDK_ROOT}/components/libraries/crypto
    ${SDK_ROOT}/components/toolchain
    ${SDK_ROOT}/components/toolchain/cmsis/include
    ${SDK_ROOT}/external/micro-ecc/micro-ecc
    ${SDK_ROOT}/components/libraries/bootloader/ble_dfu
    ${SDK_ROOT}/external/nano-pb
    ${SDK_ROOT}/components/libraries/ecc
    ${SDK_ROOT}/components/softdevice/s130/headers/nrf51
    ${SDK_ROOT}/components/libraries/queue
    ${SDK_ROOT}/components/toolchain/gcc
)

target_link_libraries(${PROJECT_NAME} PUBLIC dfu_config)
