//
// Created by vgol on 25/12/2023.
//

#include "uart.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sdk_config.h"
#include "app_uart.h"
#include "app_error.h"
#include "nrf.h"


#define UART_RX_BUF_SIZE 128
#define UART_TX_BUF_SIZE 128


extern "C" void uart_error_handle(app_uart_evt_t * p_event)
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

void uart_init(void) {

    ret_code_t err_code;

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

    while (true)
    {
        uint8_t cr;
        while (app_uart_get(&cr) != NRF_SUCCESS);
        while (app_uart_put(cr) != NRF_SUCCESS);

        if (cr == 'q' || cr == 'Q')
        {

            while (true)
            {
                // Do nothing.
            }
        }
    }

}
