cmake_minimum_required(VERSION 3.27)
project(dfu_config)


add_library(dfu_config INTERFACE)

target_include_directories(dfu_config SYSTEM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/
)


target_compile_definitions(dfu_config INTERFACE -DSOFTDEVICE_PRESENT)
target_compile_definitions(dfu_config INTERFACE -DBLE_STACK_SUPPORT_REQD)
target_compile_definitions(dfu_config INTERFACE -DSVC_INTERFACE_CALL_AS_NORMAL_FUNCTION)
target_compile_definitions(dfu_config INTERFACE -DS130)


target_compile_definitions(dfu_config INTERFACE -DNRF51)
target_compile_definitions(dfu_config INTERFACE -DNRF51422)
target_compile_definitions(dfu_config INTERFACE -DBOARD_PCA10028)
target_compile_definitions(dfu_config INTERFACE -DSWI_DISABLE0)
target_compile_definitions(dfu_config INTERFACE -DPNP_ID_PRODUCT_VERSION="${CMAKE_PROJECT_VERSION}")
target_compile_definitions(dfu_config INTERFACE -D__HEAP_SIZE=0)
target_compile_definitions(dfu_config INTERFACE -DNRF_SD_BLE_API_VERSION=2)
target_compile_definitions(dfu_config INTERFACE -DNRF_DFU_SETTINGS_VERSION=1)


#target_compile_definitions(dfu_config INTERFACE -DBL_SETTINGS_ACCESS_ONLY)
#target_compile_definitions(dfu_config INTERFACE -DNRF_DFU_SVCI_ENABLED)
#target_compile_definitions(dfu_config INTERFACE -DNRF_DFU_TRANSPORT_BLE=1)
#target_compile_definitions(dfu_config INTERFACE -DCONFIG_NFCT_PINS_AS_GPIOS)
#target_compile_definitions(dfu_config INTERFACE -DCONFIG_GPIO_AS_PINRESET)
#target_compile_definitions(dfu_config INTERFACE -DNRF_SD_BLE_API_VERSION=7)
#target_compile_definitions(dfu_config INTERFACE -DNRFX_SAADC_API_V2)
#target_compile_definitions(dfu_config INTERFACE -DAPP_TIMER_V2)
#target_compile_definitions(dfu_config INTERFACE -DSCHED_QUEUE_SIZE=40)
#target_compile_definitions(dfu_config INTERFACE -DAPP_TIMER_V2_RTC1_ENABLED)
#target_compile_definitions(dfu_config INTERFACE -DAPP_TIMER_CONFIG_USE_SCHEDULER=0)
#target_compile_definitions(dfu_config INTERFACE -DAPP_TIMER_KEEPS_RTC_ACTIVE)
#target_compile_definitions(dfu_config INTERFACE -DNRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED=1)
