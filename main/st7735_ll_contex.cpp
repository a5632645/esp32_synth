#include <string.h>
#include <esp_log.h>
#include "gui/ll_context.h"
#include "st7735_ll_contex.h"

inline static int XYToIndex(int x, int y) {
    return y * 128 + x;
}
inline static uint16_t ColorTransform(Color c) {
    return (((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3));
}

void LLContext::SetColor(int x, int y, Color c) {
    St7735LLContext* cc = (St7735LLContext*)context_;
    uint16_t* p = (uint16_t*)cc->screen_buffer + XYToIndex(x, y);
    *p = ColorTransform(c);
}

void LLContext::FillColorHorizenLine(int y, int x, int w, Color c) {
    St7735LLContext* cc = (St7735LLContext*)context_;
    uint16_t* p = (uint16_t*)cc->screen_buffer + XYToIndex(x, y);
    uint16_t color = ColorTransform(c);
    for (int i = 0; i < w; ++i) {
        *p++ = color;
    }
}

void LLContext::FillColorVeticalLine(int x, int y, int h, Color c) {
    uint16_t color = ColorTransform(c);
    St7735LLContext* cc = (St7735LLContext*)context_;
    for (int i = 0; i < h; ++i) {
        uint16_t* p = (uint16_t*)cc->screen_buffer + XYToIndex(x, y + i);
        *p = color;
    }
}

void LLContext::FillColorRect(int x, int y, int w, int h, Color c) {
    for (int i = 0; i < h; ++i) {
        FillColorHorizenLine(y + i, x, w, c);
    }
}

void LLContext::FlushScreen(int x, int y, int w, int h) {
    St7735LLContext* c = (St7735LLContext*)context_;
    LcdDrawScreen(&c->dev, (uint16_t*)c->screen_buffer, 0, 0, St7735LLContext::kWidth, St7735LLContext::kHeight);
    // if (w == St7735LLContext::kWidth && h == St7735LLContext::kHeight) {
    //     LcdDrawScreen(&c->dev, (uint16_t*)c->screen_buffer, x, y, w, h);
    // }
    // else {
    //     static constexpr int SPI_Data_Mode = 1;

    //     spi_master_write_command(&c->dev, 0x2A);	// set column(x) address
    //     spi_master_write_addr(&c->dev, x, x + w);
    //     spi_master_write_command(&c->dev, 0x2B);	// set Page(y) address
    //     spi_master_write_addr(&c->dev, y, y + h);
    //     spi_master_write_command(&c->dev, 0x2C);	//	Memory Write

    //     LcdSetSendMode(&c->dev, SPI_Data_Mode);

    //     ESP_LOGI("st7735", "x %d y %d w %d h %d", x, y, w, h);

    //     size_t max_len = 0;
    //     spi_bus_get_max_transaction_len(SPI2_HOST, &max_len);
    //     for (int i = 0; i < h; ++i) {
    //         uint16_t* colorptr = (uint16_t*)c->screen_buffer + XYToIndex(x, y + i);
    //         const uint8_t* ptr = (const uint8_t*)colorptr;
    //         size_t len = w * 2;
    //         spi_master_write_byte(c->dev._SPIHandle, ptr, len);
    //     }
    // }
}