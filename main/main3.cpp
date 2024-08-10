#include <stdio.h>
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <numbers>
#include <string>
#include <esp_timer.h>
#include "model/cordic.h"
#include "model/my_fp.h"

#include "gui/msg_queue.h"
#include "gui/component_peer.h"
#include "gui/component.h"
#include "st7735_ll_contex.h"

inline static constexpr int16_t P2FP(float x) {
    return (int16_t)(x * (1 << 14));
}
inline static constexpr float FP2P(int16_t x) {
    return x * (1.0f / (1 << 14));
}

static void TwoTask(void* arg) {
    alignas(16) int16_t x0[8] {};
    alignas(16) int16_t y0[8] {};
    alignas(16) int16_t ep[8] {};

    constexpr auto freq = std::numbers::pi_v<float> / 256.0f;
    for (int i = 0; i < 8; ++i) {
        auto w = freq * (i + 1.0f) * (128.0f/9.0f);
        auto halfw = w * 0.5f;
        auto phi = (std::numbers::pi_v<float> - w) * 0.5f;

        y0[i] = P2FP(std::sin(0.0f));
        x0[i] = P2FP(std::sin(0.0f + phi));
        ep[i] = P2FP(std::sin(halfw));
    }

    for (;;) {
        float out = 0;
        printf("((%f,%f,%f,%f,%f,%f,%f,%f,%f))\n",
         FP2P(y0[0]), FP2P(y0[1]), FP2P(y0[2]), FP2P(y0[3]), FP2P(y0[4]), FP2P(y0[5]), FP2P(y0[6]), FP2P(y0[7]), out);
        asm volatile (
            "movi.n a2,13\n\r"
            "wsr.sar a2\n\r"
            "ee.vld.128.ip q0, %[x0], 0\n\r"
            "ee.vld.128.ip q1, %[y0], 0\n\r"
            "ee.vld.128.ip q2, %[ep], 0\n\r"
            "ee.vmul.s16 q3, q2, q1\n\r"
            "ee.vsubs.s16 q0, q0, q3\n\r"
            "ee.vst.128.ip q0, %[x0], 0\n\r"
            "ee.vmul.s16 q3, q2, q0\n\r"
            "ee.vadds.s16 q1, q1, q3\n\r"
            "ee.vst.128.ip q1, %[y0], 0\n\r"
            :
            :[x0] "r"(&x0), [y0] "r"(&y0), [ep] "r"(&ep)
            :"a2"
        );
        
        vTaskDelay(pdMS_TO_TICKS(100));       
    }
}

static void TwoTask2(void*) {
    constexpr uint32_t num = 4096;
    static CoridcData* test = (CoridcData*)heap_caps_aligned_calloc(16, num, sizeof(CoridcData), MALLOC_CAP_DEFAULT);
    for (int i = 0; i < num; ++i) 
    {
        for (int j = 0; j < 8; ++j) 
        {
            test[i].gain[j] = MYFP_FROM_FLOAT(0.9f / (j + 1.0f + 8.0f * i));
        }
    }

    static CoridcResetStruct* reset = (CoridcResetStruct*)heap_caps_aligned_calloc(16, num, sizeof(CoridcResetStruct), MALLOC_CAP_DEFAULT);
    for (int i = 0; i < num; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            reset[i].freq[j] = j + 1 + 8 * i;
            reset[i].phase[j] = 0;
        }
    }
    Coridc_Reset(test, num, reset);

    // ll driver init
    static St7735LLContext ll_context;
    spi_master_init(&ll_context.dev, 2, 1, 41, 40, 42);
    lcdInit(&ll_context.dev, 128, 160, 0, 0);
    lcdDisplayOn(&ll_context.dev);

    // gui init
    Graphic g{ll_context};
    g.SetClipBoundGlobal(ll_context.GetBound());
    g.SetComponentBound(ll_context.GetBound());

    int x = 0;
    int scale = ll_context.kHeight / 2;

    CoridcFreqStruct freq {};
    MyPoint p {};
    MyPoint p2 {};

    auto tick_begin = esp_timer_get_time();
    for (;;) 
    {
        // freq.freq[0] = (freq.freq[0] + 1) & 32767;
        // Coridc_SetFreq(&test, 1, &freq);
        int32_t tmp = 0;
        Coridc_Tick(test, num, &tmp);

        auto tick_consumed = esp_timer_get_time() - tick_begin;

        auto t = MYFP_TO_FLOAT(test[num / 2].y0[0]);
        auto y = t * scale + scale;

        MyPoint pp {x, (int)y};
        g.SetColor(colors::kWhite);
        g.DrawLine(p, pp);
        p = pp;

        g.SetColor(colors::kRed);
        t = (float)tmp / INT32_MAX;
        y = t * scale + scale;
        MyPoint pp2 {x, (int)y};
        g.DrawLine(p2, pp2);
        p2 = pp2;

        // g.SetPixel({x, (int)y});
        ll_context.EndFrame(ll_context.GetBound());

        ++x;
        if (x >= ll_context.kWidth) {
            x = 0;
            g.Fill(colors::kBlack);
            g.DrawSingleLineText(std::to_string(tick_consumed) + "us", 0, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        tick_begin = esp_timer_get_time();
    }
    vTaskDelete(nullptr);
}

extern "C" void app_main() {
    // xTaskCreate(TwoTask, "TwoTask", 4096, nullptr, 5, nullptr);
    xTaskCreate(TwoTask2, "TwoTask", 4096, nullptr, 5, nullptr);
}