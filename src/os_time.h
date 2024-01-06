//
// Created by vgol on 26/12/2023.
//

#ifndef OS_TIME_H
#define OS_TIME_H

#include <stdint.h>

#define TICK_FREQ              1000uL

typedef uint32_t os_time_t;

void os_time__init();

os_time_t os_get_millis();


#endif //OS_TIME_H
