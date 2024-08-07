#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "synth_model.h"
#include "dr.h"
#include "my_fp.h"

class AddOsc {
public:
    static constexpr size_t kMaxNumHarmonics = 8;

    void Init(float sample_rate);
    void NoteOn(int note, float velocity);
    void NoteOff(int note, float velocity);
    void Process(int16_t* buffer, int len);
    bool IsPlaying() const;
    int GetNote() const;

private:
    void InternalCrTick(int len);

    float inv_sample_rate_{};
    int note_{ -1 };
    bool note_oned_{false};
    float note_freq_{};
    float output_gain_{};
    float freqs_[kMaxNumHarmonics] {};
    float gains_[kMaxNumHarmonics] {};
    float phases_[kMaxNumHarmonics] {};
    int num_active_ {};
    int dr_active_{};

    ParalleDr drs_[kMaxNumHarmonics / DR_PARALLE_SIZE] {};
};