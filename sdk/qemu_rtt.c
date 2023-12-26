//
// Created by vgol on 17/03/2023.
//

#include <stdint.h>
#include <SEGGER_RTT.h>
// #include "segger_wrapper_arm.h"

#ifdef NRF_USE_QEMU

#define DOWN_BUFFER_IDX    1u

#if USE_SVIEW
int has_sview_started = -1;

static inline void _check_bytes_avail(void) {

    const uint32_t * const p_reg = (uint32_t *)0xF0000000;
    const uint32_t * const p_val = (uint32_t *)0xF0000004;


//    if (has_sview_started == -1) { // TODO comment
//        has_sview_started = 0;
//
//        SEGGER_RTT_BUFFER_DOWN *downBuffer = &_SEGGER_RTT.aDown[DOWN_BUFFER_IDX];
//        *(downBuffer->pBuffer+downBuffer->WrOff) = 'S';
//        downBuffer->WrOff += 32;
//        return;
//    }

    uint32_t nb_buffered = *p_reg;
    if (nb_buffered) {

        // LOG_INFO("nb_buffered = %u", nb_buffered);
        for (uint32_t i=0; i< nb_buffered; i++) {
            SEGGER_RTT_BUFFER_DOWN *downBuffer = &_SEGGER_RTT.aDown[DOWN_BUFFER_IDX];

            *(downBuffer->pBuffer+downBuffer->WrOff) = (uint8_t)(*p_val);
            downBuffer->WrOff++;

            if (downBuffer->WrOff >= downBuffer->SizeOfBuffer) {
                downBuffer->WrOff = 0;
            }
        }
    }
    return;
}
#endif

// Write to fake location to pass on the log data
unsigned SEGGER_RTT_WriteNoLock_qemu(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes) {

#if USE_SVIEW
    unsigned sent = 0;
    // force checking for bytes
    if (!SEGGER_RTT_HASDATA(DOWN_BUFFER_IDX)) {
        _check_bytes_avail();
    }
    if (has_sview_started <= 0) {
        return NumBytes;
    }

    uint32_t * const p_reg = (uint32_t *)0xF0000004; // send buffer
    uint8_t *p_data = (uint8_t*) pBuffer;

    for (unsigned i = 0u; i< NumBytes && i < 512u; i++) {
        *p_reg = p_data[i];
        sent ++;
    }

    uint32_t * const p_reg_send = (uint32_t *)0xF0000008;
    *p_reg_send = 1;

    return sent;
#else

    uint32_t * const p_reg = (uint32_t *)0xF0000000; // send immediately
    uint8_t *p_data = (uint8_t*) pBuffer;

    for (unsigned i = 0; i< NumBytes; i++) {
        *p_reg = p_data[i];
    }

    return NumBytes;
#endif

}

// -serial file:mySview.bin
// -chardev id=char0,logfile=serial.log,signal=off -serial chardev:char0
// -chardev socket,id=s0,host=localhost,server,nowait,port=19111 -serial chardev:s0
unsigned SEGGER_RTT_WriteSkipNoLock_qemu(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes) {
    return SEGGER_RTT_WriteNoLock_qemu(BufferIndex, pBuffer, NumBytes);
}

#endif
