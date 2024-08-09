#include <stdio.h>
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <numbers>

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

extern "C" void app_main() {
    xTaskCreate(TwoTask, "TwoTask", 4096, nullptr, 5, nullptr);
}