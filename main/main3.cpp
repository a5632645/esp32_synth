#include <stdio.h>
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <numbers>
#include <string>
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
    CoridcData test;

    CoridcResetStruct s {
        .phase = {
            MYFP_FROM_FLOAT(0.0f),
            MYFP_FROM_FLOAT(0.0f),
            MYFP_FROM_FLOAT(0.0f),
            MYFP_FROM_FLOAT(0.0f),
            MYFP_FROM_FLOAT(0.0f),
            MYFP_FROM_FLOAT(0.0f),
            MYFP_FROM_FLOAT(0.0f),
            MYFP_FROM_FLOAT(0.0f)
        },
        .freq = {
            MYFP_FROM_FLOAT(0.01f),
            MYFP_FROM_FLOAT(0.02f),
            MYFP_FROM_FLOAT(0.03f),
            MYFP_FROM_FLOAT(0.04f),
            MYFP_FROM_FLOAT(0.05f),
            MYFP_FROM_FLOAT(0.06f),
            MYFP_FROM_FLOAT(0.07f),
            MYFP_FROM_FLOAT(0.08f)
        },
    };
    Coridc_Reset(&test, 1, &s);

    // ll driver init
    static St7735LLContext ll_context;
    spi_master_init(&ll_context.dev, 2, 1, 41, 40, 42);
    lcdInit(&ll_context.dev, 128, 160, 0, 0);
    lcdDisplayOn(&ll_context.dev);

    // gui init
    Graphic g{ll_context};
    g.SetClipBoundGlobal(ll_context.GetBound());
    g.SetComponentBound(ll_context.GetBound());
    g.SetColor(colors::kWhite);

    int x = 0;
    int scale = ll_context.kHeight / 2;

    CoridcFreqStruct freq {};
    MyPoint p {};
    for (;;) {
        // freq.freq[0] = (freq.freq[0] + 1) & 32767;
        // Coridc_SetFreq(&test, 1, &freq);
        Coridc_Tick(&test, 1, nullptr);

        auto t = MYFP_TO_FLOAT(test.y0[0]);
        auto y = t * scale + scale;

        MyPoint pp {x, (int)y};
        g.DrawLine(p, pp);
        p = pp;

        // g.SetPixel({x, (int)y});
        ll_context.EndFrame(ll_context.GetBound());

        ++x;
        if (x >= ll_context.kWidth) {
            x = 0;
            g.Fill(colors::kBlack);
            g.DrawSingleLineText(std::to_string(freq.freq[0]), 0, 0);
        }

        printf("%f\n", t);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(nullptr);
}

extern "C" void app_main() {
    // xTaskCreate(TwoTask, "TwoTask", 4096, nullptr, 5, nullptr);
    xTaskCreate(TwoTask2, "TwoTask", 4096, nullptr, 5, nullptr);
}