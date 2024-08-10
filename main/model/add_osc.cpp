#include "add_osc.h"

#include <numbers>
#include <array>
#include "synth_model.h"
#include "my_fp.h"
#include "my_math.h"

constexpr auto pi = std::numbers::pi_v<float>;
constexpr auto twopi = pi * 2.0f;

void AddOsc::Init(float sample_rate) {
    inv_sample_rate_ = 2.0f / sample_rate;
}

void AddOsc::NoteOn(int note, float velocity) {
    note_ = note;
    note_oned_ = true;
    note_freq_ = MyPitchToFreq(note) * inv_sample_rate_;
    output_gain_ = velocity;
}

void AddOsc::NoteOff(int note, float velocity) {
    note_ = -1;
    output_gain_ = 0.0f;
}

void AddOsc::Process(int16_t* buffer, int len) {
    if (!IsPlaying())
        return;

    InternalCrTick(len);
    for (int i = 0; i < len; ++i) {
        Coridc_Tick(oscs_, 1, buffer + i);
    }
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
        }
    }
    auto plus = (num_active_ & 0x7) == 0 ? 0 : 1;
    dr_active_ = (num_active_ >> 3) + plus;

    std::copy(kSawTable.cbegin(), kSawTable.cend(), std::begin(gains_));
    
    note_oned_ = false;
}
