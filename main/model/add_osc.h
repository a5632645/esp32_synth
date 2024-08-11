#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <array>
#include "synth_model.h"
#include "cordic.h"

class AddOsc {
public:
    static constexpr size_t kMaxNumHarmonics = 64;
    static constexpr size_t kNumCordic = kMaxNumHarmonics / 8;

    AddOsc();

    void Init(float sample_rate);
    void NoteOn(int note, float velocity);
    void NoteOff(int note, float velocity);
    void Process(int16_t* buffer, int len);
    bool IsPlaying() const;
    int GetNote() const;

private:
    void InternalCrTick(int len);
    void InternalFreq(int len);
    void InternalGain(int len);

    void _InharmonicLinear(uint32_t freq_inc, uint32_t max_freq);
    void _InharmonicOctave(uint32_t freq_inc, uint32_t max_freq);
    void _InharmonicString(uint32_t freq_inc, uint32_t max_freq);

    float inv_sample_rate_{};
    bool note_oned_{false};
    float note_freq_{};
    float output_gain_{};
    int note_curr_ { -1 };
    int note_prev_ { -1 };
    uint32_t num_active_ {};
    uint32_t old_num_active_ { -1 };
    uint32_t cordic_active_{};
    bool freq_changed_{};

    alignas(16) MyFpS0_15 gains_[kMaxNumHarmonics];
    std::array<CoridcData, kNumCordic> cordics_;
    std::array<CoridcParamStruct, kNumCordic> cordic_params_;

    // some parameters
    SynthModel model_ {
        .adc_vals = {255, 255, 255, 255},
        .inharmonic_mode = 255,
        .phaser_mode = 255,
        .pluck_mode = 255,
        .random_amp_mode = 255,
        .timber = 255,
        .curr_octave = 4
    };
};