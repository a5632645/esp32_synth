#include "st7735_ll_contex.h"

#include <string.h>
#include <esp_log.h>
#include <algorithm>

inline static constexpr int XYToIndex(int x, int y) {
    return y * St7735LLContext::kWidth + x;
}

inline static constexpr uint16_t ColorTransform(MyColor c) {
    // return (((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3)); // RGB565
    auto cc = c.BGR565();
    // swap high and low
    return (cc << 8) | (cc >> 8);
}

void St7735LLContext::BatchSetColor(int* x, int* y, MyColor c, int len) {
    auto color = ColorTransform(c);
    for (int i = 0; i < len; ++i)
        screen_buffer[XYToIndex(x[i], y[i])] = color;
}

void St7735LLContext::SetColor(int x, int y, MyColor c)
{
    uint16_t* p = (uint16_t*)screen_buffer + XYToIndex(x, y);
    *p = ColorTransform(c);
}

void St7735LLContext::FillColorHorizenLine(int y, int x, int w, MyColor c) {
    uint16_t* p = (uint16_t*)screen_buffer + XYToIndex(x, y);
    uint16_t color = ColorTransform(c);
    for (int i = 0; i < w; ++i) {
        *p++ = color;
    }
}

void St7735LLContext::FillColorVeticalLine(int x, int y, int h, MyColor c) {
    uint16_t color = ColorTransform(c);
    for (int i = 0; i < h; ++i) {
        uint16_t* p = (uint16_t*)screen_buffer + XYToIndex(x, y + i);
        *p = color;
    }
}

void St7735LLContext::FillColorRect(Bound aera, MyColor c) {
    for (int i = 0; i < aera.h_; ++i) {
        FillColorHorizenLine(aera.y_ + i, aera.x_, aera.w_, c);
    }
}

void St7735LLContext::FillColorHorizenLineMask(int y, int x, int w, uint8_t *mask, MyColor c) {
    uint16_t color = ColorTransform(c);
    for (int i = 0; i < w; ++i) {
        uint16_t* p = (uint16_t*)screen_buffer + XYToIndex(x + i, y);
        if (mask[i] != 0)
            *p = color;
    }
}

// void St7735LLContext::AeraPainted(Bound aera) {
//     LcdDrawScreen(&dev, (uint16_t*)screen_buffer, 0, 0, St7735LLContext::kWidth, St7735LLContext::kHeight);
//     // if (w == St7735LLContext::kWidth && h == St7735LLContext::kHeight) {
//     //     LcdDrawScreen(&c->dev, (uint16_t*)c->screen_buffer, x, y, w, h);
//     // }
//     // else {
//     //     static constexpr int SPI_Data_Mode = 1;

//     //     spi_master_write_command(&c->dev, 0x2A);	// set column(x) address
//     //     spi_master_write_addr(&c->dev, x, x + w);
//     //     spi_master_write_command(&c->dev, 0x2B);	// set Page(y) address
//     //     spi_master_write_addr(&c->dev, y, y + h);
//     //     spi_master_write_command(&c->dev, 0x2C);	//	Memory Write

//     //     LcdSetSendMode(&c->dev, SPI_Data_Mode);

//     //     ESP_LOGI("st7735", "x %d y %d w %d h %d", x, y, w, h);

//     //     size_t max_len = 0;
//     //     spi_bus_get_max_transaction_len(SPI2_HOST, &max_len);
//     //     for (int i = 0; i < h; ++i) {
//     //         uint16_t* colorptr = (uint16_t*)c->screen_buffer + XYToIndex(x, y + i);
//     //         const uint8_t* ptr = (const uint8_t*)colorptr;
//     //         size_t len = w * 2;
//     //         spi_master_write_byte(c->dev._SPIHandle, ptr, len);
//     //     }
//     // }
// }

void St7735LLContext::MoveDrawContentHorizen(Bound aera, int offset, bool left) {
    if (left) {
        for (int i = 0; i < aera.h_; ++i) {
            auto y = i + aera.y_;
            std::copy_n((uint16_t*)screen_buffer + XYToIndex(y, aera.x_ + offset),
                        aera.w_ - offset,
                        (uint16_t*)screen_buffer + XYToIndex(y, aera.x_));
        }
    }
    else {
        for (int i = 0; i < aera.h_; ++i) {
            auto y = i + aera.y_;
            auto x_src = aera.x_;
            auto x_src_end = aera.x_ + aera.w_ - offset;
            std::copy_backward((uint16_t*)screen_buffer + XYToIndex(y, x_src),
                               (uint16_t*)screen_buffer + XYToIndex(y, x_src_end),
                               (uint16_t*)screen_buffer + XYToIndex(y, aera.w_ + aera.x_));
        }
    }
}

void St7735LLContext::MoveDrawContentVetical(Bound aera, int offset, bool up) {
    auto loop_count = aera.h_ - offset;
    if (up) {
        for (int i = 0; i < loop_count; ++i)
            std::copy_n((uint16_t*)screen_buffer + XYToIndex(aera.y_ + i + offset, aera.x_),
                        aera.w_,
                        (uint16_t*)screen_buffer + XYToIndex(i + aera.y_, aera.x_));
    }
    else {
        for (int y = aera.y_ + aera.h_ - 1; y >= aera.y_ + offset; --y)
            std::copy_n((uint16_t*)screen_buffer + XYToIndex(y - offset, aera.x_),
                        aera.w_,
                        (uint16_t*)screen_buffer + XYToIndex(y, aera.x_));
    }
}

void St7735LLContext::EndFrame(Bound dirty_aera) {
    LcdDrawScreen(&dev, (uint16_t*)screen_buffer, 0, 0, St7735LLContext::kWidth, St7735LLContext::kHeight);
}
