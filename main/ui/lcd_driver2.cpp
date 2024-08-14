#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"

#include "lcd_driver2.h"

static const int SPI_Data_Mode = 1;

void LcdDriver2::Init() {
    frame_.SetBuffer(heap_caps_aligned_calloc(32, kWidth * kHeight, 2, MALLOC_CAP_DEFAULT),
                    kWidth, kHeight);
    spi_master_init(&dev_, 18, 8, 15, 17, 16, 3, 46, 0,0,0,0);
    lcdInit(&dev_, 0x9341, kWidth, kHeight, 0, 0);
    lcdDisplayOn(&dev_);
    lcdBacklightOn(&dev_);
    // lcdInversionOn(&dev_);
    // lcdBGRFilter(&dev_);
    spi_master_write_comm_byte(&dev_, 0x36);	//Memory Access Control
    spi_master_write_data_byte(&dev_, 0b01001000);	//Right top start, BGR color filter panel
}

void LcdDriver2::EndFrame(const Bound& bound) {
    spi_master_write_comm_byte(&dev_, 0x2A);	// set column(x) address
	spi_master_write_addr(&dev_, 0, kWidth-1);
	spi_master_write_comm_byte(&dev_, 0x2B);	// set Page(y) address
	spi_master_write_addr(&dev_, 0, kHeight-1);
	spi_master_write_comm_byte(&dev_, 0x2C);	//	Memory Write

    gpio_set_level( (gpio_num_t)dev_._dc, SPI_Data_Mode );

    size_t max_len = 0;
    spi_bus_get_max_transaction_len(SPI2_HOST, &max_len);
    size_t len = kWidth * kHeight * 2;
    const uint8_t* ptr = (const uint8_t*)frame_.GetPtr(0, 0);
    while(len > 0) {
        size_t len_limit = len > max_len ? max_len : len;
        spi_master_write_byte( dev_._TFT_Handle, ptr, len_limit);
        ptr += len_limit;
        len -= len_limit;
    }
}
