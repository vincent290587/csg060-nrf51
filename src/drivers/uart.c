//
// Created by vgol on 25/12/2023.
//

#define NRF_LOG_MODULE_NAME "UART"

#include "uart.h"

#include <nrf_drv_uart.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sdk_config.h"
#include "app_uart.h"
#include "app_error.h"
#include "nrf.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "os_time.h"


#define UART_RX_BUF_SIZE 32u
#define UART_TX_BUF_SIZE 32u

#define UART_PASSTHROUGH 0

#define UART_RECV_TIMEOUT_MS    ((os_time_t)10)
#define UART_SYSOFF_TIMEOUT_MS  ((os_time_t)30000)

uint32_t app_uart_put_buffer(const uint8_t * const p_buffer, size_t length);

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

enum {
    CSG060_CMD__REQUEST = 0x11,
    CSG060_CMD__SET_VALUE = 0x16,
};

enum {
    CSG060_ARG__MAX_RPM = 0x1F,
    CSG060_ARG__WHEEL_RPM = 0x20,
};

static void _handle_packet(const uint8_t * const p_buffer, size_t length) {

#if UART_PASSTHROUGH
    app_uart_put_buffer(p_buffer, length);
#else

    struct {
        uint8_t cmd;
        uint8_t arg;
        uint8_t payload[length-2];
    } * const p_data = (void*)p_buffer;

    NRF_LOG_INFO("Received controller data:\n");
    NRF_LOG_HEXDUMP_INFO(p_buffer, length);

    NRF_LOG_DEBUG("cmd: 0x%02X arg 0x%02X\n", p_data->cmd, p_data->arg);

    // handle MAX_RPM packet
    if (p_data->cmd == CSG060_CMD__SET_VALUE && p_data->arg == CSG060_ARG__MAX_RPM) {
        NRF_LOG_INFO("MAX RPM command detected: upgrading speed\n");
        const uint8_t new_buffer[] = {CSG060_CMD__SET_VALUE, CSG060_ARG__MAX_RPM, 0x00, 0xF2, 0x27};
        ret_code_t err_code = app_uart_put_buffer(new_buffer, sizeof(new_buffer));
        APP_ERROR_CHECK(err_code);
    } else {
        ret_code_t err_code = app_uart_put_buffer(p_buffer, length);
        APP_ERROR_CHECK(err_code);
    }
#endif
}

void uart_init(void) {

    ret_code_t err_code;
    os_time_t last_recv = 0;
    os_time_t last_recv_off = 0;

    const app_uart_comm_params_t comm_params =
      {
        4, // TODO
        5,
        6,
        7,
        APP_UART_FLOW_CONTROL_ENABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud1200
    };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);

    APP_ERROR_CHECK(err_code);

    uint8_t _buffer[128];
    size_t buffer_cnt = 0;

    while (true)
    {
        const os_time_t cur_time = os_get_millis();
        uint8_t cr = 0;

        err_code = app_uart_get(&cr);
        if (err_code == NRF_SUCCESS) {
            _buffer[buffer_cnt++] = cr;
            last_recv_off = last_recv = cur_time;
        } else if (last_recv && (cur_time - last_recv > UART_RECV_TIMEOUT_MS)) {

            // handle packet
            _handle_packet(_buffer, buffer_cnt);

            // clear event
            last_recv = 0;
            buffer_cnt = 0;
        }

        if (cur_time - last_recv_off > UART_SYSOFF_TIMEOUT_MS)
        {
            NRF_LOG_WARNING("Timeout, going to SYSOFF\n");
            nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
        } else {
            nrf_pwr_mgmt_run();
        }
    }

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
            err_code = app_uart_close();
            APP_ERROR_CHECK(err_code);
        break;

        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            break;

        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            break;
    }

    return true;
}

NRF_PWR_MGMT_REGISTER_HANDLER(m_app_shutdown_handler) = _app_shutdown_handler;
