//
// Created by vgol on 26/12/2023.
//

#define NRF_LOG_MODULE_NAME "TIME"

#include <stdbool.h>
#include <stdint.h>

#include "os_time.h"
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


#define TICK_FREQ              1000uL

#define RTC_PRESCALER          ((RTC_DEFAULT_CONFIG_FREQUENCY + (TICK_FREQ >> 1)) / TICK_FREQ) // rounded number


const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(0); /**< Declaring an instance of nrf_drv_rtc for RTC0. */



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
    nrf_drv_rtc_tick_enable(&rtc, false);

    nrf_drv_rtc_overflow_enable(&rtc, true);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);

    nrf_drv_rtc_disable(&rtc);

    nrf_drv_rtc_enable(&rtc);
}

uint32_t os_get_millis() {
    return nrf_drv_rtc_counter_get(&rtc);
}

void os_time__init() {
    rtc_config();
}

/**
 * @brief Handler for shutdown preparation.
 */
static bool _app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_SYSOFF:
            nrf_drv_rtc_disable(&rtc);
        break;

        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            break;

        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            break;
    }

    return true;
}

NRF_PWR_MGMT_REGISTER_HANDLER(m_app_shutdown_handler) = _app_shutdown_handler;
