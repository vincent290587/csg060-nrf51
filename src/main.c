#define NRF_LOG_MODULE_NAME "MAIN"

#include <stdbool.h>
#include <stdint.h>

#include <libraries/hardfault/hardfault.h>
#include "nrf.h"
#include "bsp.h"
#include "boards.h"
#include "uart.h"
#include "app_error.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "os_time.h"

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


static bool _app_shutdown_handler(nrf_pwr_mgmt_evt_t event);

NRF_PWR_MGMT_REGISTER_HANDLER(m_app_shutdown_handler) = _app_shutdown_handler;

/**
 * @brief Handler for shutdown preparation.
 */
bool _app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_SYSOFF:
            NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_SYSOFF\n");
        break;

        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP: {
            NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_WAKEUP\n");
            uint32_t err_code = 0;
            // TODO GPIOTE
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

    uart_init();

    ASSERT(0);

    return 0;
}
