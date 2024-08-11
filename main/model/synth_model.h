#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    uint8_t adc_vals[4]; // 0 ~ 255
    uint8_t inharmonic_mode;
    uint8_t phaser_mode;
    uint8_t pluck_mode;
    uint8_t random_amp_mode;
    
    uint8_t timber;

    int curr_octave;
} SynthModel;

extern SynthModel global_model;

#ifdef __cplusplus
}
#endif