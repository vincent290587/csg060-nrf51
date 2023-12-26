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

#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"

#define TICK_FREQ              10uL

#define RTC_PRESCALER          ((RTC_DEFAULT_CONFIG_FREQUENCY + (TICK_FREQ >> 1)) / TICK_FREQ) // rounded number


const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(0); /**< Declaring an instance of nrf_drv_rtc for RTC0. */

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

static bool _app_shutdown_handler(nrf_pwr_mgmt_evt_t event);

NRF_PWR_MGMT_REGISTER_HANDLER(m_app_shutdown_handler) = _app_shutdown_handler;

/**
 * @brief Handler for shutdown preparation.
 */
bool _app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    uint32_t err_code;

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_SYSOFF:
            NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_SYSOFF\n");
            // err_code = bsp_buttons_disable();
            nrf_drv_rtc_disable(&rtc);
            APP_ERROR_CHECK(err_code);
        break;

        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_WAKEUP\n");
            // err_code = bsp_wakeup_button_enable(BTN_ID_WAKEUP);
            APP_ERROR_CHECK(err_code);
        break;

        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_INFO("NRF_PWR_MGMT_EVT_PREPARE_DFU\n");
            APP_ERROR_HANDLER(NRF_ERROR_API_NOT_IMPLEMENTED);
        break;
    }

    return true;
}


/** @brief: Function for handling the RTC0 interrupts.
 * Triggered on TICK and COMPARE0 match.
 */
static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_COMPARE0)
    {
        NRF_LOG_INFO("NRF_DRV_RTC_INT_COMPARE0\n");
    }
    else if (int_type == NRF_DRV_RTC_INT_TICK)
    {
        NRF_LOG_INFO("NRF_DRV_RTC_INT_TICK\n");
    }
    else if (int_type == NRF_DRV_RTC_INT_OVERFLOW)
    {
        NRF_LOG_INFO("NRF_DRV_RTC_INT_TICK\n");
    }
}

/** @brief Function starting the internal LFCLK XTAL oscillator.
 */
static void lfclk_config(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}

/** @brief Function initialization and configuration of RTC driver instance.
 */
static void rtc_config(void)
{
    uint32_t err_code;

    //Initialize RTC instance
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    config.prescaler = RTC_PRESCALER - 1;
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Enable tick event & interrupt
    nrf_drv_rtc_tick_enable(&rtc, true);

    nrf_drv_rtc_overflow_enable(&rtc, false);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);

    nrf_drv_rtc_disable(&rtc);

    nrf_drv_rtc_enable(&rtc);
}

int main(void)
{
    lfclk_config();

    rtc_config();

    NRF_LOG_INIT(NULL);

    NRF_LOG_INFO("Hello world\n");

    //uart_init();

    nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);

    // This part of the example is just for testing the loopback .
    while (true)
    {
        nrf_pwr_mgmt_run();
    }

    return 0;
}
