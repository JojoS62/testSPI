/* mbed Microcontroller Library
 * Copyright (c) 2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

DigitalOut led(LED1);
SPI spi6(PB_15, PB_14, PB_13, PB_12);

int main()
{
    // DeepSleepLock lock;           // did not work when device_has SLEEP
    
    printf("Hello, Mbed!\n");

    spi6.format(16, 0);

    while(1) {
        led = !led;
        ThisThread::sleep_for(500ms);
    }
    return 0;
}