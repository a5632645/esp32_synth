#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    kInharmonicMode_Linear = 0,
    kInharmonicMode_Octave,
    kInharmonicMode_String
} InharmonicModeEnum;

typedef enum {
    kPhaserMode_D = 0,
} PhaserModeEnum;

typedef enum {
    kPluckMode_H = 0,
    kPluckMode_I,
    kPluckMode_J,
    kPluckMode_K
} PluckModeEnum;

typedef enum {
    kRandomAmpMode_L = 0,
} RandomAmpModeEnum;

typedef enum {
    kTimber_0 = 0,
    kTimber_1,
    kTimber_2
} TimberEnum;

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