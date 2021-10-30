/* mbed Microcontroller Library
 * Copyright (c) 2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

#define SPI_3WIRE
#define SPI_4WIRE

static DigitalOut user_led(LED1, 1);

int main()
{
    printf("testSPI\n");
    printf("Hello from "  MBED_STRINGIFY(TARGET_NAME) "\n");
    printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);


    PinName mosi = PB_15; // PB_5;
    [[maybe_unused]] PinName miso = PB_14;  // PB_4
    PinName sclk = PB_13;   // PC_10 (SDIO_D2) or PB_3 (JTAG_TDO / Flash SPI1_SCK)
    PinName ssel = PB_0;    // PB_11;

    DigitalOut ssel_flash(PA_15, 1);    // disable flash, on same SPI
    DigitalOut ssel_out(ssel, 1);

    int count = 0;

    size_t tx_word_len;
    size_t rx_word_len;
    const uint16_t tx_data[8] = {0x1122, 0x3344}; // allocate extra memory to prevent "out of range" error
    uint16_t rx_data[8] = {0};

    wait_us(2000);

    while (true) {
        user_led = 1;
        printf("Demo run %i\n", count);

#ifdef SPI_3WIRE
        // 3-wire demo
        // note: don't read data in 3-wire mode to get the same 4-wire results
        tx_word_len = 2;
        rx_word_len = 0;
        {
            SPI spi(mosi, NC, sclk);
            spi.format(16, 0);

            ssel_out = 0;

            // transfer by 1 word per SPI::write call
            for (size_t i = 0; i < tx_word_len; i++) {
                spi.write(tx_data[i]);
            }
            wait_us(64);

            // transfer by bulk SPI::write call
            spi.write((const char *)tx_data, tx_word_len * sizeof(*tx_data),
                      (char *)rx_data, rx_word_len * sizeof(*rx_data));
            wait_us(64);
            
            // asynchronous transfer
            spi.transfer((const char *)tx_data, tx_word_len * sizeof(*tx_data),
                         (char *)rx_data, rx_word_len * sizeof(*rx_data), nullptr);
            wait_us(64);

            ssel_out = 1;
        }
#endif

#ifdef SPI_4WIRE
        // 4-wire demo
        tx_word_len = 2;
        rx_word_len = 2;
        {
            SPI spi(mosi, miso, sclk);
            spi.format(16, 0);

            wait_us(64);

            ssel_out = 0;

            // transfer by 1 word per SPI::write call
            for (size_t i = 0; i < tx_word_len; i++) {
                spi.write(tx_data[i]);
            }
            wait_us(64);

            // transfer by bulk SPI::write call
            spi.write((const char *)tx_data, tx_word_len * sizeof(*tx_data),
                      (char *)rx_data, rx_word_len * sizeof(*rx_data));
            wait_us(64);
            
            // asynchronous transfer
            spi.transfer(tx_data, tx_word_len * sizeof(*tx_data),
                         rx_data, rx_word_len * sizeof(*rx_data), nullptr);
            wait_us(64);

            ssel_out = 1;
        }
#endif

        ThisThread::sleep_for(100ms);
        user_led = 0;
        ThisThread::sleep_for(1900ms);
        count++;
    }

    return 0;
}
