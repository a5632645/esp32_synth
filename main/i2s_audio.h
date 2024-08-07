#pragma once

#include <driver/i2s_std.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void(*callback)(void* buffer, int len);
    int i2s_port;
    int sample_rate;
    int bck_gpio;
    int ws_gpio;
    int out_gpio;
    i2s_slot_mode_t channel_count;
} I2sAudioConfigT;

void I2sAudioInit(const I2sAudioConfigT* pconfig);

#ifdef __cplusplus
}
#endif