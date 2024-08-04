#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "synth_model.h"
#include "dr.h"

class AddOsc {
public:
    static constexpr size_t kMaxNumHarmonics = 8;

    void Init(float sample_rate);
    void NoteOn(int note, float velocity);
    void NoteOff(int note, float velocity);
    void Process(float* buffer, int len);
    bool IsPlaying() const;
    int GetNote() const;

private:
    void InternalCrTick(int len);
    inline float TickOnce() noexcept {
        float output = 0.0f;
        for (int i = 0; i < num_active_; ++i) {
            sins_[i].Tick();
            output += sins_[i].Sin() * gains_[i];
        }
        return output;
    }

    float inv_sample_rate_{};
    int note_{ -1 };
    bool note_oned_{false};
    float note_freq_{};
    float output_gain_{};
    float freqs_[kMaxNumHarmonics] {};
    float gains_[kMaxNumHarmonics] {};
    int num_active_ {};
    DR<float> sins_[kMaxNumHarmonics];
};