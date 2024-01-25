#define NRF_LOG_MODULE_NAME "MAIN"

#include <app_scheduler.h>
#include <app_timer_appsh.h>
#include <nrf_delay.h>
#include <stdbool.h>
#include <stdint.h>

#include <libraries/hardfault/hardfault.h>
#include "nrf.h"
#include "bsp.h"
#include "dfu_mgmt.h"
#include "boards.h"
#include "uart.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_wdt.h"
#include "nrf_drv_gpiote.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "os_time.h"

#ifdef BLE_STACK_SUPPORT_REQD
#include <ble_app.h>
#include <ble_stack_handler_types.h>
#else
#define BLE_STACK_HANDLER_SCHED_EVT_SIZE 0
#endif

#define SCHED_MAX_EVENT_DATA_SIZE       MAX(APP_TIMER_SCHED_EVT_SIZE, BLE_STACK_HANDLER_SCHED_EVT_SIZE)               /**< Maximum size of scheduler events. */

#ifdef SVCALL_AS_NORMAL_FUNCTION
#define SCHED_QUEUE_SIZE                 20                                         /**< Maximum number of events in the scheduler queue. More is needed in case of Serialization. */
#else
#define SCHED_QUEUE_SIZE                10                                                        /**< Maximum number of events in the scheduler queue. */
#endif

#define WAKE_PIN    UART_RX


nrf_drv_wdt_channel_id m_channel_id;

/**
 * Function is implemented as weak so that it can be overwritten by custom application error handler
 * when needed.
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_ERROR("app_error_fault_handler: \n");
    NRF_LOG_ERROR("0x%08X - pc 0x%08X \n", id, pc, info);

    assert_info_t * p_assert_info = (assert_info_t *)info;
    error_info_t * p_error_info = (error_info_t *)info;

    switch (id) {
        case NRF_FAULT_ID_SDK_ERROR:
            NRF_LOG_ERROR("ERROR 0x%08X at %s:%u",
                    p_error_info->err_code,
                    (char*)p_error_info->p_file_name,
                    p_error_info->line_num);
            break;

        case NRF_FAULT_ID_SDK_ASSERT:
            NRF_LOG_ERROR("ASSERTION FAILED at %s:%u",
                          (char*)p_assert_info->p_file_name,
                          p_assert_info->line_num);
            break;

        default:
            break;
    }

    // On assert, the system can only recover with a reset.
#ifndef DEBUG
    NVIC_SystemReset();
#else
    app_error_save_and_stop(id, pc, info);
#endif // DEBUG
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * file_name)
{
    assert_info_t assert_info =
    {
        .line_num    = line_num,
        .p_file_name = file_name,
    };
    app_error_fault_handler(NRF_FAULT_ID_SDK_ASSERT, 0, (uint32_t)(&assert_info));

    UNUSED_VARIABLE(assert_info);
}

static uint32_t _wakeup_prepare(void) { // configure new interrupt

    // https://github.com/NordicPlayground/nrf51-powerdown-examples/blob/master/system_off_wakeup_on_gpiote/main.c

    // Configure to generate interrupt and wakeup on pin signal low. "false" means that gpiote will use the PORT event,
    // which is low power, i.e. does not add any noticable current consumption (<<1uA).
    // Setting this to "true" will make the gpiote module use GPIOTE->IN events which add ~8uA for nRF52 and ~1mA for nRF51.
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(false);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    ret_code_t err_code = nrf_drv_gpiote_in_init(WAKE_PIN, &in_config, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(WAKE_PIN, true);

    return err_code;
}

/**
 * @brief Handler for shutdown preparation.
 */
static bool _app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_SYSOFF:
            NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_SYSOFF\n");
        break;

        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP: {
            NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_WAKEUP\n");
            uint32_t err_code = _wakeup_prepare();
            APP_ERROR_CHECK(err_code);
        }
        break;

        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_DFU\n");
        break;
    }

    return true;
}

NRF_PWR_MGMT_REGISTER_HANDLER(m_app_shutdown_handler) = _app_shutdown_handler;

/**@brief Function for handling HardFault.
 */
void HardFault_process(HardFault_stack_t *p_stack)
{
    for (;;)
    {
        // No implementation needed.
#ifdef DEBUG
    app_error_save_and_stop(0, p_stack->pc, 2);
#else
    // Restart the system by default
    NVIC_SystemReset();
#endif
    }
}

/**
 * @brief WDT events handler.
 */
static void wdt_event_handler(void) {

}

static void _wdt_enable(void) {
#if WDT_ENABLED
    //Configure WDT.
    ret_code_t err_code;
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();
#endif
}

static void _wait_func(void) {
#if WDT_ENABLED
    nrf_drv_wdt_channel_feed(m_channel_id);
#endif
    app_sched_execute();
#ifdef BLE_STACK_SUPPORT_REQD
    sd_app_evt_wait();
#else
    nrf_pwr_mgmt_run();
#endif

}

int main(void)
{
    _wdt_enable();

    os_time__init();

    NRF_LOG_INIT(os_get_millis);

    NRF_LOG_INFO("Hello CSG060\n");

    nrf_pwr_mgmt_init(TICK_FREQ);

    dfu_mgmt__init();

    ret_code_t err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    //Initialize output pin
    const nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(true); //Configure output LED
    err_code = nrf_drv_gpiote_out_init(LED_PIN_APP, &out_config);
    APP_ERROR_CHECK(err_code);

    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);

    ble_app__init();

    NRF_LOG_INFO("APP ready\n");

#if 1
    uart_init(_wait_func);
#else
    while (os_get_millis() < 20000) {
        _wait_func();
    }
#endif

#ifdef DEBUG
    int count = 50;
    while (count--) {
        nrf_drv_gpiote_out_toggle(LED_PIN_APP);
        nrf_delay_ms(50);
    }
#endif
    nrf_drv_gpiote_out_clear(LED_PIN_APP);

    NRF_LOG_WARNING("Timeout, going to SYSOFF\n");
    NRF_LOG_FLUSH();

#if WDT_ENABLED
    nrf_drv_wdt_channel_feed(m_channel_id);
#endif

    nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    while (true) {
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_CONTINUE);

#if WDT_ENABLED
        nrf_drv_wdt_channel_feed(m_channel_id);
        app_sched_execute();
#endif
    }

    return 0;
}
