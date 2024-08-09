#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cmath>
#include <numbers>
#include "model/dr.h"

static void DrTest(void*) {
    ParalleDr dr;

    constexpr auto freq = 0.001f;
    
    MyFpFloatBundleT fb;
    MyFpFloatBundleT fb2;
    for (int i = 0; i < 8; ++i) {
        auto p = freq * (i + 1.0f) * std::numbers::pi_v<float>;
        fb.f32[i] = std::sin(p);
        fb2.f32[i] = 2.0f * std::cos(p);
    }

    dr.sin0_ = MYFP_FROM_FLOAT_BC(0.0f);
    // dr.sin1_ = MYFP_FROM_FLOAT_BC(0.30901699437494742410229341718282f);
    // dr.coeff_ = MYFP_FROM_FLOAT_BC(2.0f * 0.95105651629515357211643933337938f);
    MyFp_FromFloatBundle(&fb, &dr.sin1_);
    MyFp_FromFloatBundle(&fb2, &dr.coeff_);

    for (int i = 0; i < 8; ++i)
        fb.f32[i] = std::numbers::inv_pi_v<float> / (i + 1.0f);
    MyFp_FromFloatBundle(&fb, &dr.gain_);

    for (;;) {
        float out = 0;

        MyFpInt128T i128;
        MyFp_Mul(&dr.sin0_, &dr.gain_, &i128);
        for (int i = 0; i < 8; ++i)
            out += MYFP_TO_FLOAT(i128.s16[i]);

        MyFp_ToFloatBundle(&dr.sin0_, &fb);
        printf("((%f,%f,%f,%f,%f,%f,%f,%f,%f))\n", fb.f32[0], fb.f32[1], fb.f32[2], fb.f32[3], fb.f32[4], fb.f32[5], fb.f32[6], fb.f32[7], out);
        asm volatile (
            "movi.n a2, 13\n\r"
            "wsr.sar a2\n\r"
            "ee.vld.128.ip q0, %[sin0], 0\n\r"
            "ee.vld.128.ip q1, %[sin1], 0\n\r"
            "ee.vld.128.ip q2, %[coeff],0\n\r"
            // "ee.vld.128.ip q4, %[gain], 0\n\r"
            "ee.vmul.s16 q3, q1, q2\n\r"
            "ee.vsubs.s16 q3, q3, q0\n\r"
            "ee.vst.128.ip q3, %[sin1], 0\n\r"
            "ee.vst.128.ip q1, %[sin0], 0\n\r"
            // "ee.vmulas.s16.accx q4, q1\n\r"
            // "movi.n a2, 14\n\r"
            // "ee.srs.accx a3, a2, 0\n\r"
            // "s32i a3, %[out], 0\n\r"
            :
            :[sin0]"r"(&dr.sin0_),[sin1]"r"(&dr.sin1_),[coeff]"r"(&dr.coeff_)/*,[out]"r"(&out),[gain]"r"(&dr.gain_)*/
            :"a2","a3"
        );

        // auto e = (dr.sin1_.s16[0] * dr.coeff_.s16[0] >> 13) - dr.sin0_.s16[0];
        // dr.sin0_.s16[0] = dr.sin1_.s16[0];
        // dr.sin1_.s16[0] = e;
        // e = (dr.sin1_.s16[1] * dr.coeff_.s16[1] >> 13) - dr.sin0_.s16[1];
        // dr.sin0_.s16[1] = dr.sin1_.s16[1];
        // dr.sin1_.s16[1] = e;
        // printf("((%f,%f))\n", MYFP_TO_FLOAT(dr.sin0_.s16[0]), MYFP_TO_FLOAT(dr.sin0_.s16[1]));
        
        vTaskDelay(pdMS_TO_TICKS(100));       
    }
}

extern "C" void app_main() {
    xTaskCreate(DrTest, "dr_test", 4096, nullptr, 4, nullptr);
}