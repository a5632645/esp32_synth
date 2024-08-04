#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float adc_vals[4];
    int inharmonic_mode;
    int phaser_mode;
    int pluck_mode;
    int random_amp_mode;

    int curr_octave;
} SynthModel;

extern SynthModel global_model;

#ifdef __cplusplus
}
#endif