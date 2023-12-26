#define NRF_LOG_MODULE_NAME "MAIN"

#include <stdbool.h>
#include <stdint.h>

#include <libraries/hardfault/hardfault.h>
#include "nrf.h"
#include "bsp.h"
#include "boards.h"
#include "uart.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "os_time.h"

#define WAKE_PIN 6

/**
 * Function is implemented as weak so that it can be overwritten by custom application error handler
 * when needed.
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_ERROR("Fatal\r\n");
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

    nrf_drv_gpiote_in_config_t in_config;
    in_config.is_watcher = false;
    in_config.hi_accuracy = true;
    in_config.pull = NRF_GPIO_PIN_PULLUP;
    in_config.sense = NRF_GPIOTE_POLARITY_HITOLO;

    ret_code_t err_code = nrf_drv_gpiote_in_init(WAKE_PIN, &in_config, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(WAKE_PIN, true);

    nrf_gpio_cfg_sense_input(WAKE_PIN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
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
            APP_ERROR_HANDLER(NRF_ERROR_API_NOT_IMPLEMENTED);
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


int main(void)
{

    os_time__init();

    NRF_LOG_INIT(os_get_millis);

    NRF_LOG_INFO("Hello CSG060\n");

    const ret_code_t err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    uart_init();

    ASSERT(0);

    return 0;
}
