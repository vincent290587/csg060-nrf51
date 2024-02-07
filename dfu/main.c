/**
 * Copyright (c) 2016 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/** @file
 *
 * @defgroup bootloader_secure main.c
 * @{
 * @ingroup dfu_bootloader_api
 * @brief Bootloader project main file for secure DFU.
 *
 */

#define NRF_LOG_MODULE_NAME "DFU_MAIN"

#include <app_scheduler.h>
#include <dfu_mgmt_types.h>
#include <nrf_dfu_types.h>
#include <nrf_dfu_settings.h>
#include <stdint.h>
#include <nrf_drv_gpiote.h>
#include <nrf_drv_ppi.h>

#include "boards.h"
#include "nrf_mbr.h"
#include "nrf_bootloader.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_dfu.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_error.h"
#include "app_error_weak.h"
#include "nrf_bootloader_info.h"

#define USE_PPI 1

nrf_ppi_channel_t ppi_channel;

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_ERROR("received a fault! id: 0x%08x, pc: 0x&08x\r\n", id, pc);
    NVIC_SystemReset();
}

void app_error_handler_bare(uint32_t error_code)
{
    (void)error_code;
    NRF_LOG_ERROR("received an error: 0x%08x!\r\n", error_code);
    NVIC_SystemReset();
}

_Static_assert (UART_RX != UART_TX, "Wrong pin selection");

static void _on_dfu_start() {

    if (NRF_WDT->RUNSTATUS) {
        NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    }

    ret_code_t err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    //Initialize output pin
    const nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(true); //Configure output LED
    err_code = nrf_drv_gpiote_out_init(LED_PIN, &out_config);                       //Initialize output button
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_set(LED_PIN);

    const nrf_drv_gpiote_out_config_t out_config2 = GPIOTE_CONFIG_OUT_TASK_TOGGLE(true); //Configure output TX
    err_code = nrf_drv_gpiote_out_init(UART_TX, &out_config2);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_set(UART_TX);

    const nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);

    err_code = nrf_drv_gpiote_in_init(UART_RX, &in_config, NULL);
    APP_ERROR_CHECK(err_code);

#if USE_PPI

    err_code =  nrf_drv_ppi_channel_alloc(&ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(ppi_channel ,
        nrf_drv_gpiote_in_event_addr_get(UART_RX),
        nrf_drv_gpiote_out_task_addr_get(UART_TX)
        );
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_enable(ppi_channel);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(UART_RX, false);
    nrf_drv_gpiote_out_task_enable(UART_TX);

#endif
}

static void _on_dfu_end() {

#if USE_PPI

    nrf_drv_gpiote_in_event_enable(UART_RX, false);
    nrf_drv_gpiote_out_task_disable(UART_TX);

    ret_code_t err_code = nrf_drv_ppi_channel_disable(ppi_channel);
    APP_ERROR_CHECK(err_code);

    nrf_drv_ppi_channel_free(ppi_channel);

#endif

    nrf_drv_gpiote_out_clear(LED_PIN);

    nrf_drv_gpiote_uninit();
}

static void _check_timeout() {

    volatile uint32_t counter = NRF_RTC0->COUNTER; // 512 seconds overflow
    if (counter > (32768uL * 300uL)) { // 300 seconds timeout
        NRF_LOG_INFO("Timeout !");
        NVIC_SystemReset();
    }
}

extern void __real_app_sched_execute();

void __wrap_app_sched_execute()
{
    // Transport is waiting for event?
    while(true)
    {
        if (NRF_WDT->RUNSTATUS) {
            NRF_WDT->RR[0] = WDT_RR_RR_Reload;
        }
        _check_timeout();
        // Can't be emptied like this because of lack of static variables
        __real_app_sched_execute();
    }
}

extern fs_ret_t __real_nrf_dfu_flash_erase(uint32_t const * p_dest, uint32_t num_pages, dfu_flash_callback_t callback);

fs_ret_t __wrap_nrf_dfu_flash_erase(uint32_t const * p_dest, uint32_t num_pages, dfu_flash_callback_t callback) {

    if (NRF_WDT->RUNSTATUS) {
        NRF_WDT->RR[0] = WDT_RR_RR_Reload;
    }
    return __real_nrf_dfu_flash_erase(p_dest, num_pages, callback);
}

extern void __real_nrf_bootloader_app_start(uint32_t start_addr);

void __wrap_nrf_bootloader_app_start(uint32_t start_addr) {

    _on_dfu_end();

    __real_nrf_bootloader_app_start(start_addr);
}

extern nrf_dfu_settings_t s_dfu_settings;

nrf_dfu_action_t m_dfu_action __attribute__ ((section(".noinit")))
                              __attribute__((used));

bool nrf_dfu_enter_check(void)
{
    NRF_LOG_DEBUG("In user nrf_dfu_enter_check\n");

    if (m_dfu_action.enter_buttonless_dfu && m_dfu_action.passcode == DFU_PASSCODE) {

        NRF_LOG_WARNING("Found DFU passcode\n");

        m_dfu_action.enter_buttonless_dfu = 0;
        m_dfu_action.passcode = 0;
        return true;
    } else {

        NRF_LOG_INFO("DFU passcode: 0x%08lX\n", m_dfu_action.passcode);
    }

    NRF_LOG_FLUSH();

    m_dfu_action.enter_buttonless_dfu = 0;
    m_dfu_action.passcode = 0;

    if (s_dfu_settings.enter_buttonless_dfu == 1)
    {
        s_dfu_settings.enter_buttonless_dfu = 0;
        APP_ERROR_CHECK(nrf_dfu_settings_write(NULL));
        return true;
    }
    return false;
}

#define DECLARE_SECTION(section)    \
extern unsigned int __ ## section ## _src, __ ## section ## _start, __ ## section ## _end;
#define STARTUP_SECTION(section)    StartupSection(&__ ## section ## _start, &__ ## section ## _end)

DECLARE_SECTION(rttSection)

static void StartupSection(volatile unsigned int* dst, const unsigned int* dst_end)
{
    // Initialize the zero init section
    for (; dst < dst_end; dst++) {
        *dst = 0;
    }
}

/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t ret_val;

    STARTUP_SECTION(rttSection);

    NRF_LOG_INIT(NULL);

    NRF_LOG_INFO("Inside main\n");

    NRF_LOG_INFO("Softdevice size 0x%lX \n", SD_SIZE_GET(MBR_SIZE));
    NRF_LOG_INFO("Softdevice FWID 0x%lX \n", SD_FWID_GET(MBR_SIZE));

    _on_dfu_start();

    ret_val = nrf_bootloader_init();
    APP_ERROR_CHECK(ret_val);

    // Either there was no DFU functionality enabled in this project or the DFU module detected
    // no ongoing DFU operation and found a valid main application.
    // Boot the main application.
    nrf_bootloader_app_start(MAIN_APPLICATION_START_ADDR);

    // Should never be reached.
    NRF_LOG_INFO("After main\r\n");
}

/**
 * @}
 */
