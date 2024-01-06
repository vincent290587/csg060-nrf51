//
// Created by vgol on 06/01/2024.
//

#ifndef DFU_MGMT_TYPES_H
#define DFU_MGMT_TYPES_H

#include <stdint.h>

#define DFU_PASSCODE 0xCAFEDECA

typedef struct
{
    uint32_t            passcode;
    uint32_t            enter_buttonless_dfu;
} nrf_dfu_action_t;

#endif //DFU_MGMT_TYPES_H
