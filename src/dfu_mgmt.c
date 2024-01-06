//
// Created by vgol on 06/01/2024.
//

#define NRF_LOG_MODULE_NAME "DFU"

#include <nrf_pwr_mgmt.h>

#include "dfu_mgmt.h"
#include "dfu_mgmt_types.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"


nrf_dfu_action_t m_dfu_action __attribute__ ((section(".noinit")))
                              __attribute__((used));

void dfu_mgmt__init(void) {

    m_dfu_action.enter_buttonless_dfu = 0;
    m_dfu_action.passcode = 0;
}

static void dfu_mgmt__enter_dfu(void) {

    NRF_LOG_INFO("Preparing for starting DFU\n");

    m_dfu_action.passcode = DFU_PASSCODE;
    m_dfu_action.enter_buttonless_dfu = 1;
}

/**
 * @brief Handler for shutdown preparation.
 */
static bool _app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    uint32_t err_code;

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_SYSOFF:
            break;

        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            break;

        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            dfu_mgmt__enter_dfu(); // indicate we want to enter DFU
            return true;
            break;
    }

    return true;
}

NRF_PWR_MGMT_REGISTER_HANDLER(m_app_shutdown_handler) = _app_shutdown_handler;

