//
// Created by vgol on 25/12/2023.
//

#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*p_wait_func_t)(void);

void uart_init(p_wait_func_t pFunc);

#ifdef __cplusplus
}
#endif

#endif //UART_H
