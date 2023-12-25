#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "nrf.h"
#include "bsp.h"
#include "uart.h"


int main(void)
{
    bsp_board_leds_init();

    uart_init();

    // This part of the example is just for testing the loopback .
    while (true)
    {
        ;
    }

    return 0;
}
