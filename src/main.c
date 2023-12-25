#include <nrf_log_ctrl.h>
#include <stdbool.h>
#include <stdint.h>
#include <libraries/hardfault/hardfault.h>

#include "nrf.h"
#include "bsp.h"
#include "uart.h"
#include "app_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
/*lint -save -e14 */

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
    bsp_board_leds_init();

    NRF_LOG_INIT(NULL);

    NRF_LOG_INFO("Hello world\r\n");

    //uart_init();

    // This part of the example is just for testing the loopback .
    while (true)
    {
        ;
    }

    return 0;
}
