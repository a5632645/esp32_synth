#include "synth_model.h"

SynthModel global_model = {
    .adc_vals = {0, 0, 0, 0},
    .curr_octave = 4,
    .inharmonic_mode = 0,
    .phaser_mode = 0,
    .pluck_mode = 0,
    .random_amp_mode = 0,
    .timber = 0
};