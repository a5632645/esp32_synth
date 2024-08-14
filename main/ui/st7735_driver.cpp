#include "st7735_driver.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>

void St7735Driver::Init() {
    frame_.SetBuffer(heap_caps_aligned_calloc(32, kWidth * kHeight, 2, MALLOC_CAP_DEFAULT),
                    kWidth, kHeight);
    spi_master_init(&dev_, 2, 1, 41, 40, 42);
    lcdInit(&dev_, 128, 160, 0, 0);
    lcdDisplayOn(&dev_);
}

void St7735Driver::BeginFrame() {
}

void St7735Driver::AeraDrawed(const Bound &bound) {
}

void St7735Driver::EndFrame(const Bound& bound) {
    LcdDrawScreen(&dev_, (uint16_t*)frame_.GetPtr(0,0), 0, 0, kWidth, kHeight);
}
