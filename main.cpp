/* mbed Microcontroller Library
 * Copyright (c) 2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "stm32h7xx_ll_spi.h"

// #define SPI_3WIRE
#define SPI_4WIRE

static DigitalOut user_led(LED1, 1);

#ifdef TARGET_STM32H7
/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(SPI_HandleTypeDef &hspi2)
{
    /* SPI2 parameter configuration*/
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 0x0;
    hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        //Error_Handler();
    }
}

static SPI_TypeDef *staticSPI;

class MySPI : public SPI {
    public:
    MySPI(PinName mosi, PinName miso, PinName sclk, PinName ssel = NC) :
        SPI(mosi, miso, sclk, ssel)
    {
        struct spi_s *spiobj = (( struct spi_s *)(&_peripheral->spi));
    }

    void reinit() {
        spi_t *obj = &_peripheral->spi;
        struct spi_s *spiobj = (( struct spi_s *)(&_peripheral->spi));
        SPI_HandleTypeDef *handle = &(spiobj->handle);

        staticSPI = ((SPI_TypeDef *)(obj->spi.spi));

        LL_SPI_StartMasterTransfer(staticSPI);

        //MX_SPI2_Init(*handle);
    }

    static inline void spi_transmit(uint32_t data)
    {
        LL_SPI_TransmitData16(staticSPI, data);
        while(!LL_SPI_IsActiveFlag_TXP(staticSPI)) {};
//        LL_SPI_ReceiveData8(staticSPI); /* dummy read-access to clear SPI_SR_RXWNE */
    }

    static inline uint32_t spi_receive(uint32_t data)
    {
        LL_SPI_TransmitData8(staticSPI, data);
        while(!LL_SPI_IsActiveFlag_TXP(staticSPI)) {};
        while(!LL_SPI_IsActiveFlag_RXWNE(staticSPI)) {};
        return LL_SPI_ReceiveData8(staticSPI);
    }

};
#else
#define MySPI SPI
#endif

void spiSendDone(int event)
{
    user_led = !user_led;
}

int main()
{
    DeepSleepLock   lock();

    printf("testSPI\n");
    printf("Hello from "  MBED_STRINGIFY(TARGET_NAME) "\n");
    printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);


    PinName mosi = PB_15; // PB_5;
    [[maybe_unused]] PinName miso = PB_14;  // PB_4
    PinName sclk = PB_13;   // PC_10 (SDIO_D2) or PB_3 (JTAG_TDO / Flash SPI1_SCK)
    PinName ssel = PB_0;    // PB_11;

    DigitalOut ssel_out(ssel, 1);
    DigitalOut ssel_flash(PA_15, 1);    // disable flash, on same SPI

    int count = 0;

    size_t tx_word_len;
    size_t rx_word_len;
    const uint8_t tx_data8[8] = {0x11, 0x22, 0x33, 0x44}; // allocate extra memory to prevent "out of range" error
    const uint16_t tx_data[8] = {0x1122, 0x3344}; // allocate extra memory to prevent "out of range" error
    uint16_t rx_data[8] = {0};

    wait_us(2000);

    constexpr uint spi_freq = 40'000'000;

            MySPI spi(mosi, miso, sclk);
            spi.format(16, 0);
            spi.frequency(spi_freq);
            spi.reinit();

    while (true) {
        //user_led = 1;
        printf("Demo run %i\n", count);

#ifdef SPI_3WIRE
        // 3-wire demo
        // note: don't read data in 3-wire mode to get the same 4-wire results
        tx_word_len = 2;
        rx_word_len = 0;
        {
            SPI spi(mosi, NC, sclk);
            spi.format(16, 0);
            spi.frequency(spi_freq);

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

#ifdef SPI_4WIRE
        // 4-wire demo
        tx_word_len = 2;
        rx_word_len = 2;
        {

            wait_us(64);

            ssel_out = 0;
            // transfer by 1 word per SPI::write call
            for (size_t i = 0; i < tx_word_len; i++) {
                spi.spi_transmit(tx_data[i]);
            }
            wait_us(64);


            // // transfer by 1 word per SPI::write call
            // for (size_t i = 0; i < tx_word_len; i++) {
            //     spi.write(tx_data[i]);
            // }
            // wait_us(64);

            // // transfer by bulk SPI::write call
            // spi.write((const char *)tx_data, tx_word_len * sizeof(*tx_data),
            //           (char *)rx_data, rx_word_len * sizeof(*rx_data));
            // wait_us(64);
            
            // // asynchronous transfer
            // spi.transfer(tx_data, tx_word_len * sizeof(*tx_data),
            //              rx_data, rx_word_len * sizeof(*rx_data), spiSendDone);
            // wait_us(64);

            ssel_out = 1;
        }
#endif

        ThisThread::sleep_for(500ms);
        // user_led = 0;
        // ThisThread::sleep_for(1900ms);
        count++;
    }

    return 0;
}
