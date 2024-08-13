#include <stdio.h>
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <numbers>
#include <string>
#include <esp_timer.h>
#include <esp_log.h>

#include "gui/msg_queue.h"
#include "gui/component_peer.h"
#include "gui/component.h"
#include "st7735_ll_contex.h"

#include "model/dr.h"

static void TwoTask2(void*) {
    DrData test;

    alignas(16) int16_t phase[8] {};
    alignas(16) int16_t freq[8] {
        MyFpS0_15_FromFloat(0.1f),
        MyFpS0_15_FromFloat(0.2f),
        MyFpS0_15_FromFloat(0.3f),
        MyFpS0_15_FromFloat(0.4f),
        MyFpS0_15_FromFloat(0.5f),
        MyFpS0_15_FromFloat(0.6f),
        MyFpS0_15_FromFloat(0.7f),
        MyFpS0_15_FromFloat(0.8f)
    };
    alignas(16) int16_t amp[8] {
        MyFpS0_15_FromFloat(1.0f),
        MyFpS0_15_FromFloat(1.0f / 2.0f),
        MyFpS0_15_FromFloat(1.0f / 3.0f),
        MyFpS0_15_FromFloat(1.0f / 4.0f),
        MyFpS0_15_FromFloat(1.0f / 5.0f),
        MyFpS0_15_FromFloat(1.0f / 6.0f),
        MyFpS0_15_FromFloat(1.0f / 7.0f),
        MyFpS0_15_FromFloat(1.0f / 8.0f)
    };

    Dr_Reset(&test, 1, freq, phase);

    // // ll driver init
    // static St7735LLContext ll_context;
    // spi_master_init(&ll_context.dev, 2, 1, 41, 40, 42);
    // lcdInit(&ll_context.dev, 128, 160, 0, 0);
    // lcdDisplayOn(&ll_context.dev);

    // // gui init
    // Graphic g{ll_context};
    // g.SetClipBoundGlobal(ll_context.GetBound());
    // g.SetComponentBound(ll_context.GetBound());

    // int x = 0;
    // int scale = ll_context.kHeight / 2;

    // MyPoint p {};
    // MyPoint p2 {};
    // float freqf = 0.1f;
    // auto tick_begin = esp_timer_get_time();
    for (;;) 
    {
        // int32_t tmp = 0;
        // Dr_Tick(&test, 1, amp, &tmp);
        // printf("%f,%f,%f,%f,%f,%f,%f,%f\n", MyFpS1_14_ToFloat(test.sin0[0]),
        //     MyFpS1_14_ToFloat(test.sin0[1]),
        //     MyFpS1_14_ToFloat(test.sin0[2]),
        //     MyFpS1_14_ToFloat(test.sin0[3]),
        //     MyFpS1_14_ToFloat(test.sin0[4]),
        //     MyFpS1_14_ToFloat(test.sin0[5]),
        //     MyFpS1_14_ToFloat(test.sin0[6]),
        //     MyFpS1_14_ToFloat(test.sin0[7]));
        for (int i = 0; i < 8; ++i) {       
            printf(i == 7? "%f\n" : "%f,", MyFpS1_14_ToFloat(test.sin0[i]));
            auto e = (test.sin1[i] * test.half_coeff[i] >> 14) - test.sin0[i];
            test.sin0[i] = test.sin1[i];
            test.sin1[i] = e;
        }

        // auto tick_consumed = esp_timer_get_time() - tick_begin;

        // auto t = MyFpS1_14_ToFloat(test.sin0[0]);
        // auto y = t * scale + scale;

        // MyPoint pp {x, (int)y};
        // g.SetColor(colors::kWhite);
        // g.DrawLine(p, pp);
        // p = pp;

        // g.SetColor(colors::kRed);
        // t = (float)tmp / (1 << 25);
        // y = t * scale + scale;
        // MyPoint pp2 {x, (int)y};
        // g.DrawLine(p2, pp2);
        // p2 = pp2;

        // ll_context.EndFrame(ll_context.GetBound());

        // ++x;
        // if (x >= ll_context.kWidth) {
        //     x = 0;
        //     g.Fill(colors::kBlack);
        //     g.DrawSingleLineText(std::to_string(tick_consumed) + "us", 0, 0);

        //     freqf += 0.01f;
        //     if (freqf > 0.9f)
        //         freqf = 0.1f;
        //     freq[0] = MyFpS1_14_FromFloat(freqf);
        //     Dr_SetFreq(&test, 1, freq);
        //     g.DrawSingleLineText(std::to_string(freqf), 0, 8);
        // }

        vTaskDelay(10);
        // tick_begin = esp_timer_get_time();
    }
    vTaskDelete(nullptr);
}

extern "C" void app_main() {
    // xTaskCreate(TwoTask, "TwoTask", 4096, nullptr, 5, nullptr);
    xTaskCreate(TwoTask2, "TwoTask", 4096, nullptr, 5, nullptr);
}