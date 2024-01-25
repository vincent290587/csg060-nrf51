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

#define UART_RECV_TIMEOUT_MS    ((os_time_t)15)
#define UART_SYSOFF_TIMEOUT_MS  ((os_time_t)60000)

extern uint32_t app_uart_put_buffer(const uint8_t * const p_buffer, size_t length);

static void uart_error_handle(app_uart_evt_t * p_event)
{
#ifdef DEBUG
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        NRF_LOG_ERROR("APP_UART_COMMUNICATION_ERROR\n");
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        NRF_LOG_ERROR("AAPP_UART_FIFO_ERROR\n");
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
#endif
}

// https://github.com/EBiCS/EBiCS_Firmware/blob/master/Src/display_bafang.c

enum {
    CSG060_CMD__LEVEL = 0x0B,
    CSG060_CMD__STARTREQUEST = 0x11,
    CSG060_CMD__STARTINFO = 0x16,
    CSG060_CMD__LIGHT = 0x1A,
};

enum {
    CSG060_ARG__MAX_RPM = 0x1F,
    CSG060_ARG__WHEEL_RPM = 0x20,
};

static void _handle_packet(const uint8_t * const p_buffer, size_t length) {

#if UART_PASSTHROUGH
    NRF_LOG_INFO("Received controller data:\n");
    NRF_LOG_HEXDUMP_INFO(p_buffer, length);
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
    if (p_data->cmd == CSG060_CMD__STARTINFO && p_data->arg == CSG060_ARG__MAX_RPM) {
        NRF_LOG_INFO("MAX RPM command detected: upgrading speed\n");
        // Default value is BD hex which is 189 rpm. Wheel diameter for a 700C-38 tire is around 2.18 mtrs
        // This leads to a top speed of 189 * 2.18 * 60 / 1000 kph = 24.7 kph
        const uint8_t new_buffer[] = {CSG060_CMD__STARTINFO, CSG060_ARG__MAX_RPM, 0x00, 0xF2, 0x27}; // 16h+1Fh+00h+F2h = 27h
        ret_code_t err_code = app_uart_put_buffer(new_buffer, sizeof(new_buffer));
        APP_ERROR_CHECK(err_code);
    } else if (p_data->cmd == CSG060_CMD__STARTREQUEST || p_data->cmd == CSG060_CMD__STARTINFO) {
        switch (p_data->cmd) {
            case CSG060_CMD__LEVEL:
                NRF_LOG_INFO("CSG060_CMD__LEVEL\n");
            break;
            case CSG060_CMD__LIGHT:
                NRF_LOG_INFO("CSG060_CMD__LIGHT\n");
            break;
            default:
                break;
        }
        const ret_code_t err_code = app_uart_put_buffer(p_buffer, length);
        APP_ERROR_CHECK(err_code);
    } else {
        NRF_LOG_WARNING("Not forwarding bytes: wrong header\n");
    }
#endif
}

void uart_init(p_wait_func_t pFunc) {

    ret_code_t err_code;
    os_time_t last_recv = 0;
    os_time_t last_recv_off = 0;

    const app_uart_comm_params_t comm_params = {
        .rx_pin_no = UART_RX, // D0 is TX
        .tx_pin_no = UART_TX, // D1 is RX
        NRF_UART_PSEL_DISCONNECTED,
        NRF_UART_PSEL_DISCONNECTED,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud1200 // byte duration: 8.333 ms
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

    NRF_LOG_INFO("Ready for bytes\n");

    while (true)
    {
        const os_time_t cur_time = os_get_millis();
        uint8_t cr = 0;

        err_code = app_uart_get(&cr);
        if (err_code == NRF_SUCCESS && buffer_cnt < sizeof(_buffer)) {
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
            err_code = app_uart_flush();
            APP_ERROR_CHECK(err_code);
            err_code = app_uart_close();
            APP_ERROR_CHECK(err_code);
            return;
        } else {
            pFunc();
        }
    }

}
