//
// Created by vgol on 26/12/2023.
//

#define NRF_LOG_MODULE_NAME "TIME"

#include <stdbool.h>
#include <stdint.h>

#include "os_time.h"

#include <app_timer.h>
#include <app_timer_appsh.h>

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

#if APP_TIMER_ENABLED==0

#define RTC_PRESCALER          ((RTC_DEFAULT_CONFIG_FREQUENCY + (TICK_FREQ >> 1)) / TICK_FREQ) // rounded number


const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(0); /**< Declaring an instance of nrf_drv_rtc for RTC0. */


/** @brief: Function for handling the RTC0 interrupts.
 * Triggered on TICK and COMPARE0 match.
 */
static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_COMPARE0)
    {
        // NRF_LOG_INFO("NRF_DRV_RTC_INT_COMPARE0\n");
    }
    else if (int_type == NRF_DRV_RTC_INT_TICK)
    {
        // NRF_LOG_INFO("NRF_DRV_RTC_INT_TICK\n");
    }
    else if (int_type == NRF_DRV_RTC_INT_OVERFLOW)
    {
        // NRF_LOG_INFO("NRF_DRV_RTC_INT_OVERFLOW\n");
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

    lfclk_config();

    //Initialize RTC instance
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    config.prescaler = RTC_PRESCALER - 1;
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Enable tick event & interrupt
    nrf_drv_rtc_tick_enable(&rtc, true);

    nrf_drv_rtc_overflow_enable(&rtc, true);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}

uint32_t os_get_millis() {
    return nrf_drv_rtc_counter_get(&rtc);
}

void os_time__init() {
    rtc_config();
#ifdef DEBUG
    while (!os_get_millis()) {

    }
#endif
}

/**
 * @brief Handler for shutdown preparation.
 */
static bool _app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_SYSOFF:
            nrf_drv_rtc_tick_enable(&rtc, false);
            nrf_drv_rtc_disable(&rtc);
            // nrf_drv_clock_lfclk_release();
            break;

        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            break;

        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            nrf_drv_rtc_tick_enable(&rtc, false);
            nrf_drv_rtc_disable(&rtc);
            // nrf_drv_clock_lfclk_release();
            break;
    }

    return true;
}

NRF_PWR_MGMT_REGISTER_HANDLER(m_app_shutdown_handler) = _app_shutdown_handler;

#else

#define APP_TIMER_PRESCALER             0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                          /**< Size of timer operation queues. */

#define TIMEOUT_TICKS                   10

#define APP_TICK_EVENT_INTERVAL         APP_TIMER_TICKS(TIMEOUT_TICKS, APP_TIMER_PRESCALER)

APP_TIMER_DEF(m_tick_timer);

static volatile uint32_t m_ticks;

static void tick_timeout_handler(void * p_context) {
    m_ticks += TIMEOUT_TICKS;
}


/** @brief Function starting the internal LFCLK XTAL oscillator.
 */
static void lfclk_config(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}

void os_time__init() {

    lfclk_config();

    // Initialize timer module, making it use the scheduler.
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

    app_timer_create(&m_tick_timer, APP_TIMER_MODE_REPEATED, tick_timeout_handler);

    app_timer_start(m_tick_timer, APP_TICK_EVENT_INTERVAL, NULL);
}

uint32_t os_get_millis() {
    return m_ticks;
}

#endif
