#include "add_osc.h"

#include <numbers>
#include <array>
#include "synth_model.h"

constexpr auto pi = std::numbers::pi_v<float>;
constexpr auto twopi = pi * 2.0f;

void AddOsc::Init(float sample_rate) {
    inv_sample_rate_ = 1.0f / sample_rate;
}

void AddOsc::NoteOn(int note, float velocity) {
    note_ = note;
    note_oned_ = true;
    note_freq_ = MyPitchToFreq(note) * inv_sample_rate_ * twopi;
    output_gain_ = velocity;
}

void AddOsc::NoteOff(int note, float velocity) {
    note_ = -1;
    output_gain_ = 0.0f;
}

void AddOsc::Process(float* buffer, int len) {
    if (!IsPlaying())
        return;

    InternalCrTick(len);
    for (int i = 0; i < len; ++i)
        buffer[i] += TickOnce() * output_gain_;
}

bool AddOsc::IsPlaying() const {
    return note_ != -1;
}

int AddOsc::GetNote() const {
    return note_;
}

constexpr auto kSawTable = [] {
    std::array<float, AddOsc::kMaxNumHarmonics> t{};
    for (int i = 0; i < AddOsc::kMaxNumHarmonics; ++i)
        t[i] = (1.0f / pi) / (i + 1.0f);
    return t;
}();

void AddOsc::InternalCrTick(int len) {
    // auto cr_phase_inc = len * inv_sample_rate_;

    num_active_ = kMaxNumHarmonics;
    for (int i = 0; i < kMaxNumHarmonics; ++i) {
        freqs_[i] = note_freq_ * (1.0f + i);
        if (freqs_[i] > pi) {
            num_active_ = i;
            break;
        }
    }

    std::copy(kSawTable.cbegin(), kSawTable.cend(), std::begin(gains_));

    if (note_oned_)
        for (int i = 0; i < num_active_; ++i)
            sins_[i].Reset(freqs_[i], 0.0f);
    else
        for (int i = 0; i < num_active_; ++i)
            sins_[i].SetFreq(freqs_[i]);
    
    note_oned_ = false;
}
